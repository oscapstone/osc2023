#include "thread.h"
#include "ds/list.h"
#include "utils.h"
#include <stdbool.h>
#include "interrupt.h"
#include "peripherals/mini_uart.h"

// 8 different pri
struct ds_list_head run_queue;
struct ds_list_head zombie_queue;
static bool _inited = 0;
static uint64_t thread_num = 0;

#define CHECK_ZNUM 3

struct RegState_t kernel_state;

void thread_clean(struct Thread_t *th) {

    kfree(th->sp - DEFAULT_STACK_SZ);
    kfree(th->arg);
    ds_list_remove(&(th->th_head));
    process_free(th->proc);
    kfree(th);
}

void thread_check_zombie(void * arg) {
    while(true) {
        struct ds_list_head *front = ds_list_front(&zombie_queue);
        for(int i = 0; i < CHECK_ZNUM && front != NULL; i ++) {
            uint64_t flag = interrupt_disable_save();
            struct Thread_t *th = container_of(front, struct Thread_t, tq_head);
            ds_list_remove(&(th->tq_head));

            interrupt_enable_restore(flag);
            front = ds_list_front(&zombie_queue);
            asm volatile("nop");
        }
    }
}
void thread_control_init() {
    if(_inited) {
        return;
    }
    _inited = 1;
    ds_list_head_init(&run_queue);
    ds_list_head_init(&zombie_queue);
}


extern void back_el1();
void thread_exit() {

    back_el1();
    struct Thread_t *th = thread_get_current_instance();
    th->status = TH_ZOMBIE;
    

    // wait for interrupt
    // This make sure that the scheudle is called in el1
    enable_interrupt();
    while(1) {
        asm volatile("nop");
    }
}


void _thread_exit() {
    struct ds_list_head *front = ds_list_front(&(run_queue));
    // front should not be NULL, since we have idle thread
    struct Thread_t *th = container_of(front, struct Thread_t, tq_head);
    thread_exit();
}

void sys_call_exit() {
    asm volatile(
        "mov x0, 5\n"
        "svc 0\n"
    );
    return;
}

void thread_exec_wrapper() {

    uint64_t flag = interrupt_disable_save();
    struct Thread_t *cur_thread = thread_get_current_instance();
    interrupt_enable_restore(flag);
    cur_thread->entry(cur_thread->arg);

    flag = interrupt_disable_save();
    thread_exit();
    interrupt_enable_restore(flag);
}

struct Thread_t * thread_create(thread_func func, void *arg) {
    struct Thread_t *th = (struct Thread_t *)kmalloc(sizeof(struct Thread_t));
    if(arg == NULL) {
        th->arg = NULL;
    } else {
        uint32_t arg_len = strlen(arg);
        th->arg = (void *)kmalloc(arg_len + 1);
        memcpy(th->arg, arg, arg_len);
        *(char*)(th->arg + arg_len) = '\0';
    }
    // TID may races,
    uint64_t flag = interrupt_disable_save();
    th->tid = ++ thread_num;
    th->entry = func;
    // 4KB stack
    ds_list_head_init(&(th->tq_head));
    ds_list_head_init(&(th->th_head));
    th->sp = (void *)kmalloc(DEFAULT_STACK_SZ) + (DEFAULT_STACK_SZ);
    th->stack_sz = DEFAULT_STACK_SZ;
    th->status = TH_IN_RUNNING;
    ds_list_addprev(&run_queue, &(th->tq_head));
    memset(&th->saved_reg, 0, sizeof(struct RegState_t));
    th->saved_reg.fp = th->sp;
    th->saved_reg.sp = th->sp;
    // this should make the function end will jump to cleanup function
    th->saved_reg.lr = thread_exec_wrapper;
    th->saved_reg.tcb_addr = th;
    interrupt_enable_restore(flag);
    return th;
}
static uint64_t switch_time;

void schedule_init() {
    uint64_t freq, clock;
    asm volatile("mrs %0, cntfrq_el0\n":"=r"(freq));
    asm volatile("mrs %0, cntpct_el0\n":"=r"(clock));
    freq >>= 5;
    asm volatile("msr cntp_tval_el0, %0": "=r"(freq));
    switch_time = freq + clock;
}


static uint8_t first_schedule = 1;

void schedule(uint8_t preempt) {
    uint64_t flag = interrupt_disable_save();
    struct ds_list_head *front = ds_list_front(&(run_queue));
    // front should not be NULL, since we have idle thread
    struct Thread_t *cur_thread = container_of(front, struct Thread_t, tq_head);
    ds_list_remove(&(cur_thread->tq_head));
    ds_list_head_init(&(cur_thread->tq_head));


    //setup next timer
    uint64_t freq;
    if(preempt) {
        asm volatile("mrs %0, cntfrq_el0\n":"=r"(freq));
        freq >>= 5;
        asm volatile("msr cntp_tval_el0, %0": "=r"(freq));
    }

    if(first_schedule) {
        cur_thread->status = TH_RUNNING;
        first_schedule = 0;
        // Make first context switch push front
        // so that the status of the run queue is correct
        // namely, the first thread instance should be running.
        ds_list_addnext(&run_queue, &(cur_thread->tq_head));
        enable_interrupt();
        context_switch_to(&(kernel_state), &(cur_thread->saved_reg), cur_thread->tid);
        return;
    }


    else {
        front = ds_list_front(&(run_queue));
        if(front == NULL) {
            goto _r;
        }
        struct Thread_t *next_thread = container_of(front, struct Thread_t, tq_head);
        while(next_thread->status != TH_IN_RUNNING) {

            if(next_thread == cur_thread) {
                break;
            }
            ds_list_remove(&(next_thread->tq_head));
            ds_list_head_init(&(next_thread->tq_head));
            if(next_thread == TH_WAIT) {
                if(next_thread->waitqueue != NULL) {
                    ds_list_addprev(&(next_thread->waitqueue->th_list), &(next_thread->tq_head));
                }
            } else if(next_thread->status == TH_ZOMBIE) {
                ds_list_addprev(&(zombie_queue), &(next_thread->tq_head));
            }
            front = ds_list_front(&(run_queue));
            if(front == NULL) {
                goto _r;
            }
            next_thread = container_of(front, struct Thread_t, tq_head);
        }
        if(cur_thread->status != TH_RUNNING) {
            if(cur_thread->status == TH_WAIT) {
                if(cur_thread->waitqueue != NULL) {
                    ds_list_addprev(&(cur_thread->waitqueue->th_list), &(cur_thread->tq_head));
                }
            } else if(cur_thread->status == TH_ZOMBIE) {
                ds_list_addprev(&(zombie_queue), &(cur_thread->tq_head));
            } else{
            }
        } else{
            cur_thread->status = TH_IN_RUNNING;
            ds_list_addprev(&(run_queue), &(cur_thread->tq_head));
        }
        next_thread->status = TH_RUNNING;



        enable_interrupt();
        context_switch_to(&(cur_thread->saved_reg), &(next_thread->saved_reg), next_thread->tid);
        signal_check();
        return;
    }
    _r:

    ds_list_addprev(&(run_queue), &(cur_thread->tq_head));
    interrupt_enable_restore(flag);
    signal_check();
    return;
}

struct Thread_t *thread_get_current_instance() {
    struct ds_list_head *front = ds_list_front(&(run_queue));
    struct Thread_t *cur_thread = container_of(front, struct Thread_t, tq_head);
    return cur_thread;
}

void handle_time_schedule(struct Trapframe_t *frame) {
    // return;
    uint64_t freq, clock;
    asm volatile("mrs %0, cntfrq_el0\n":"=r"(freq));
    asm volatile("mrs %0, cntpct_el0\n":"=r"(clock));

    if(switch_time > clock) {
        return;
    }
    switch_time = clock + (freq >> 5);
    struct Thread_t *cur_th = thread_get_current_instance();
    // Since we won't do eret after schedule, we need to enable the interrupt first
    schedule(1);
}