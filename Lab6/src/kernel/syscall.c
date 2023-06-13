#include "stdlib.h"
#include "thread.h"
#include "mini_uart.h"
#include "page_alloc.h"
#include "reserve_mem.h"
#include "read_cpio.h"
#include "utils.h"
#include "peripherals/mbox_call.h"
#include "peripherals/gpio.h"
#include "my_signal.h"

extern task_struct *get_current();
extern void set_switch_timer();
extern void enable_interrupt();
extern void disable_interrupt();
extern void core_timer_enable();
extern void switch_to(task_struct *, task_struct *);
extern task_struct kernel_thread;
extern struct list_head task_rq_head;
extern struct list_head task_zombieq_head;

int getpid()
{
    int ret = get_current()->thread_info->id;
    return ret;
}

size_t uart_read(char buf[], size_t size)
{
    for (int i = 0; i < size; i++)
    {
        buf[i] = uart_recv();
        if (buf[i] == '\n' || buf[i] == '\r')
            return i;
    }
    return size;
}

size_t uart_write(const char buf[], size_t size)
{
    for (int i = 0; i < size; i++)
    {
        if (buf[i] == '\0')
            return i;
        uart_send(buf[i]);
    }
    return size;
}

int exec(const char *name, char *const argv[])
{
    task_struct *current = get_current();

    // Change to kernel virtual address
    current->usrpgm_load_addr = KERNEL_PA_TO_VA(virtual_mem_translate((void *)current->usrpgm_load_addr));
    current->ustack_start = KERNEL_PA_TO_VA(virtual_mem_translate((void *)current->ustack_start));

    load_userprogram(name, (void *)current->usrpgm_load_addr);

    // Map custom virtual address to dynamic allocated address
    // Note that my_malloc() return virtual (with kernel prefix), map_pages() remove it for physical
    uint64_t *pgd = (uint64_t *)KERNEL_VA_TO_PA(new_page_table());
    map_pages(pgd, DEFAULT_THREAD_VA_CODE_START, (uint64_t)current->usrpgm_load_addr, USRPGM_SIZE / PAGE_SIZE); // map for code space
    map_pages(pgd, DEFAULT_THREAD_VA_STACK_START, (uint64_t)current->ustack_start, 4);                          // map for stack

    // Use virtual address instead
    current->usrpgm_load_addr = (unsigned long)DEFAULT_THREAD_VA_CODE_START;
    current->ustack_start = (unsigned long)DEFAULT_THREAD_VA_STACK_START;
    unsigned long target_addr = current->usrpgm_load_addr;
    unsigned long target_sp = current->ustack_start + MIN_PAGE_SIZE * 4;

    // Change ttbr0_el1
    current->task_context.ttbr0_el1 = (uint64_t)pgd;
    write_gen_reg(x0, current->task_context.ttbr0_el1);
    asm volatile("dsb ish");           // ensure write has completed
    asm volatile("msr ttbr0_el1, x0"); // switch translation based address.
    asm volatile("tlbi vmalle1is");    // invalidate all TLB entries
    asm volatile("dsb ish");           // ensure completion of TLB invalidatation
    asm volatile("isb");               // clear pipeline

    current->usrpgm_load_addr_pa = (unsigned long)virtual_mem_translate((void *)DEFAULT_THREAD_VA_CODE_START);
    current->ustack_start_pa = (unsigned long)virtual_mem_translate((void *)DEFAULT_THREAD_VA_STACK_START);

    // printf("target_addr = %p\n", target_addr);
    // printf("virtual_mem_translate(target_addr) = %p\n", virtual_mem_translate(target_addr));
    // printf("target_sp = %p\n", target_sp);
    // printf("virtual_mem_translate(target_sp) = %p\n", virtual_mem_translate(target_sp));

    set_switch_timer();
    core_timer_enable();
    enable_interrupt();

    asm volatile(
        "mov x0, 0x0\n\t" // EL0t, and open diaf(interrupt)
        "msr spsr_el1, x0\n\t"
        "mov x0, %0\n\t"
        "msr elr_el1, x0\n\t"
        "mov x0, %1\n\t"
        "msr sp_el0, x0\n\t"
        "eret"
        :
        : "r"(target_addr), "r"(target_sp)
        : "x0");

    return 0;
}

int fork()
{
    schedule();
    return get_current()->thread_info->child_id;
}

void exit(int status)
{
    task_struct *current = (task_struct *)get_current();
    current->status = ZOMBIE;
    INIT_LIST_HEAD(&current->list);
    list_add_tail(&current->list, &task_zombieq_head);
    // schedule();
    switch_to(current, &kernel_thread);
}

int mbox_call_u(unsigned char ch, unsigned int *mbox)
{
    unsigned int r = (((unsigned int)((unsigned long)mbox) & ~0xF) | (ch & 0xF));

    /* wait until we can write to the mailbox */
    while (1)
    {
        if (!(get32(MBOX_STATUS) & MBOX_FULL))
            break;
    }

    /* write the address of our message to the mailbox with channel identifier */
    put32(MBOX_WRITE, r);

    /* now wait for the response */
    while (1)
    {
        /* is there a response? */
        while (1)
        {
            if (!(get32(MBOX_STATUS) & MBOX_EMPTY))
                break;
        }

        /* is it a response to our message? */
        if (r == get32(MBOX_READ))
            /* is it a valid successful response? */
            return mbox[1] == MBOX_RESPONSE;
    }

    return 0;
}

void kill(int pid)
{
    task_struct *tmp = get_current();
    if (tmp->thread_info->id == pid)
        exit(0);

    struct list_head *iter = &task_rq_head;
    struct list_head *start = &task_rq_head;
    while (iter->next != start)
    {
        iter = iter->next;
        tmp = container_of(iter, task_struct, list);
        if (tmp->thread_info->id == pid)
        {
            tmp->status = ZOMBIE;
            return;
        }
    }
}

void signal(int SIGNAL, void (*handler)())
{
    printf("[info] Called signal()\n");
    task_struct *cur = get_current();

    custom_signal *new = (custom_signal *)my_malloc(sizeof(custom_signal));
    new->sig_num = SIGNAL;
    new->handler = handler;
    INIT_LIST_HEAD(&new->list);

    if (!cur->custom_signal)
        cur->custom_signal = new;
    else
        list_add_tail(&cur->custom_signal->list, &new->list);
}