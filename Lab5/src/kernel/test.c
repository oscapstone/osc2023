#include "stdlib.h"
#include "dynamic_alloc.h"
#include "page_alloc.h"
#include "thread.h"
#include "syscall.h"
#include "read_cpio.h"
#include "utils.h"
#include "timer.h"

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
    return;
}

void load_usrpgm_in_umode()
{
    task_struct *current = get_current();
    unsigned long target_addr = current->usrpgm_load_addr;
    unsigned long target_sp = current->ustack_start + MIN_PAGE_SIZE;

    load_userprogram("syscall.img", (char *)target_addr);

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