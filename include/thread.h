#ifndef __THREAD_H
#define __THREAD_H
#include "process.h"
#include "waitqueue.h"

#define DEFAULT_STACK_SZ ((1 << 14))
typedef void (*thread_func)(void*);
typedef enum THStatus_{
    TH_IN_RUNNING=0,
    TH_RUNNING,
    TH_WAIT,
    TH_EXIT,
    TH_ZOMBIE
} THStatus;
struct RegState_t {
    uint64_t x0;
    uint64_t x1;
    uint64_t x2;
    uint64_t x3;
    uint64_t x4;
    uint64_t x5;
    uint64_t x6;
    uint64_t x7;
    uint64_t x8;
    uint64_t x9;
    uint64_t x10;
    uint64_t x11;
    uint64_t x12;
    uint64_t x13;
    uint64_t x14;
    uint64_t x15;
    uint64_t x16;
    uint64_t x17;
    uint64_t x18;
    uint64_t x19;
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
    uint64_t fp;
    uint64_t lr;
    uint64_t sp;
    uint64_t ttbr0_el1;
    uint64_t pad;
    uint64_t tcb_addr;
};
struct Thread_t{
    uint64_t tid;
    THStatus status;
    struct Process_t *proc;
    void *arg;
    // for adding thread to run_queue or wait_queue
    struct ds_list_head tq_head;
    // for adding thread to process list
    struct ds_list_head th_head;
    thread_func entry;
    void *sp;
    void *sig_sp;
    uint64_t stack_sz;
    struct RegState_t saved_reg;
    struct waitqueue_t* waitqueue;
};
struct Thread_t * thread_create(thread_func func, void *arg);
void schedule(uint8_t preempt);
void thread_control_init();
struct Thread_t *thread_get_current_instance();
void handle_time_schedule(struct Trapframe_t *frame);
void schedule_init();
void thread_exec_wrapper();
#endif