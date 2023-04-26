#include "sched.h"
#include "ctx.h"
#include "list.h"
#include "signal.h"
#include "memory.h"
#include "string.h"
#include "interrupt.h"
#include "timer.h"
#include "uart.h"

list_head_t * ready_queue;
list_head_t * wait_queue;
thread_t * cur_thread;
thread_t threads[PIDMAX + 1];

void init_sched_thread() {
    // uart_printf("dbg: init_sched_thread start\n");
    enter_critical();

    ready_queue = malloc(sizeof(list_head_t));
    INIT_LIST_HEAD(ready_queue);
    wait_queue = malloc(sizeof(list_head_t));
    INIT_LIST_HEAD(wait_queue);

    for (int i = 0; i <= PIDMAX; i++) {
        threads[i].id = i;
        threads[i].status = FREE;
    }

    // aaa
    cur_thread = malloc(sizeof(thread_t));
    set_current_ctx(&cur_thread->context);

    thread_create(idle);

    exit_critical();
    // uart_printf("dbg: init_sched_thread end\n");
}

void idle() {
    // uart_printf("dbg: idle start\n");
    while (1) {
        kill_zombies();
        schedule();
    }
    // uart_printf("dbg: idle end\n");
}

int timer_list_get_size()
{
    int r = 0;
    struct list_head *curr;
    list_for_each(curr, ready_queue)
    {
        r++;
    }
    return r;
}

void schedule() {
    enter_critical();
    // uart_printf("dbg: schedule total %d - %d", timer_list_get_size(), cur_thread->id);

    do {
        cur_thread = (thread_t *)cur_thread->listhead.next;
    } while (list_is_head(&cur_thread->listhead, ready_queue) || cur_thread->status == DEAD);

    switch_to(current_ctx, &(cur_thread->context));
    // uart_printf("dbg: schedule end\n");

    exit_critical();
}

void kill_zombies() {
    enter_critical();

    list_head_t * iter;
    thread_t * tmp;
    list_for_each(iter, ready_queue) {
        tmp = (thread_t *)iter;
        if (tmp->status == DEAD) {
            list_del_entry(iter);
            free(tmp->user_sp);
            free(tmp->kernel_sp);
            tmp->status = FREE;
        }
    }

    exit_critical();
}

thread_t * thread_create(void * start) {
    // uart_printf("dbg: thread_create staet\n");
    enter_critical();

    thread_t * tmp;
    for (int i = 0; i <= PIDMAX; i++) {
        if (threads[i].status == FREE) {
            tmp = &(threads[i]);
            break;
        }
    }

    tmp->context.lr = (unsigned long long)start;
    tmp->status = READY;
    tmp->user_sp = malloc(USTACK_SIZE);
    tmp->kernel_sp = malloc(KSTACK_SIZE);
    tmp->context.sp = (unsigned long long)tmp->kernel_sp + KSTACK_SIZE;
    tmp->context.fp = tmp->context.sp;

    tmp->signal_is_checking = 0;
    // initial signal handler with signal_default_handler (kill thread)
    for (int i = 0; i < SIGNAL_MAX; i++) {
        tmp->signal_handler[i] = signal_default_handler;
        tmp->sigcount[i] = 0;
    }

    list_add_tail(&tmp->listhead, ready_queue);

    exit_critical();
    // uart_printf("dbg: thread_create end\n");

    return tmp;
}

int thread_exec(char * data, unsigned int filesize) {
    // uart_printf("dbg: thread_exec start\n");
    thread_t * tmp = thread_create(data);
    tmp->data = malloc(filesize);
    tmp->datasize = filesize;
    tmp->context.lr = (unsigned long)tmp->data;
    memcpy(tmp->data, data, filesize);
    
    cur_thread = tmp;

    add_timer(schedule_timer, "", get_current_tick() + get_clock_freq());

    asm volatile(
        "msr tpidr_el1, %0\n\t"
        "msr elr_el1, %1\n\t"
        "msr spsr_el1, xzr\n\t"
        "msr sp_el0, %2\n\t"
        "mov sp, %3\n\t"
        "eret\n\t" ::"r"(&tmp->context),
        "r"(tmp->context.lr), "r"(tmp->user_sp + USTACK_SIZE), "r"(tmp->kernel_sp + KSTACK_SIZE)
    );
   
    // uart_printf("dbg: thread_exec end\n");
    return 0;
}

void thread_exit() {
    enter_critical();
    cur_thread->status = DEAD;
    exit_critical();
    schedule();
}

void schedule_timer(char * notuse) {
    unsigned long long cntfrq_el0 = get_clock_freq();
    add_timer(schedule_timer, "", get_current_tick() + (cntfrq_el0 >> 5));
}