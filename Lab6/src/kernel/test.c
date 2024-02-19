#include "stdlib.h"
#include "dynamic_alloc.h"
#include "page_alloc.h"
#include "thread.h"
#include "syscall.h"
#include "read_cpio.h"
#include "utils.h"
#include "timer.h"
#include "reserve_mem.h"

extern task_struct *get_current();
extern void set_switch_timer();
extern void enable_interrupt();
extern void disable_interrupt();
extern void core_timer_enable();

void test_mem_alloc()
{
    void *a;
    a = my_malloc(200);
    printf("a = %p\n", a);
    free(a);

    a = my_malloc(200);
    printf("a = %p\n", a);
    free(a);

    a = my_malloc(200);
    printf("a = %p\n", a);
    free(a);

    a = my_malloc(200);
    printf("a = %p\n", a);
    free(a);

    a = my_malloc(200);
    printf("a = %p\n", a);
    free(a);

    return;
}

void foo()
{
    for (int i = 0; i < 10; ++i)
    {
        task_struct *cur = get_current();
        printf("Thread id: %d %d\n", cur->thread_info->id, i);
        delay(1000000);
        schedule();
    }
}

void test_thread()
{
    int N = 5;
    for (int i = 0; i < N; ++i)
    { // N should > 2
        thread_create(foo);
    }

    // thread_create(load_usrpgm_in_umode);
    idle_task();

    debug_task_rq();
    debug_task_zombieq();
    return;
}

void load_usrpgm_in_umode()
{
    task_struct *current = get_current();

    load_userprogram("vm.img", (void *)current->usrpgm_load_addr);

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
}