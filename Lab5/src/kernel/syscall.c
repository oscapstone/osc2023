#include "thread.h"
#include "mini_uart.h"
#include "page_alloc.h"
#include "read_cpio.h"
#include "utils.h"
#include "peripherals/mbox_call.h"
#include "peripherals/gpio.h"

#include "stdlib.h"

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
    unsigned long target_addr = current->usrpgm_load_addr;
    unsigned long target_sp = current->ustack_start + MIN_PAGE_SIZE;

    set_switch_timer();
    core_timer_enable();
    enable_interrupt();

    load_userprogram(name, (char *)target_addr);

    asm volatile(
        "mov x0, 0x0\n\t" // EL0t
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
    unsigned int r = (((unsigned int)((unsigned long)&mbox) & ~0xF) | (ch & 0xF));

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
