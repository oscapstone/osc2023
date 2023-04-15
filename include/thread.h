#ifndef _THREAD_H
#define _THREAD_H
#include "stdint.h"
#include "types.h"
#include "list.h"

#define TASK_RUNNING            0
#define TASK_INTERRUPTIBLE      1
#define TASK_UNINTERRUPTIBLE    2
#define __TASK_STOPPED          4
#define __TASK_TRACED           8

/* in tsk->exit_state */
#define EXIT_DEAD               16
#define EXIT_ZOMBIE             32
#define EXIT_TRACE              (EXIT_ZOMBIE | EXIT_DEAD)

/* in tsk->state again */
#define TASK_DEAD               64
#define TASK_WAKEKILL           128    /** wake on signals that are deadly **/
#define TASK_WAKING             256
#define TASK_PARKED             512
#define TASK_NOLOAD             1024
#define TASK_STATE_MAX          2048
struct task_reg_set {
    uint64_t x19, x20;
    uint64_t x21, x22;
    uint64_t x23, x24;
    uint64_t x25, x26;
    uint64_t x27, x28;
    uint64_t fp, lr;
    uint64_t sp;
};
typedef struct task_struct {
    union {
        struct list_head node;
    };
    volatile long state;
    pid_t tid;
    // pid_t tgid;
    // pid_t ppid;
    char *kernel_stack;
    size_t kernel_stack_size;
    char *user_stack;
    size_t user_stack_size;
    char *text;
    
    struct task_reg_set old_reg_set;

    //about signal
    int exit_state;
    int exit_code;
    int exit_signal;
    //signal handler table
    //TODO
} task_t;
//prepare a template for create_thread
//Notice that this function Call kmalloc!!!
extern task_t *new_thread();
//create a new thread
extern task_t *create_thread(char *text);
extern void destruct_thread(task_t *t);
extern list_t running_queue, waiting_queue, stop_queue;
//When the current thread calls this API, the scheduler picks the next thread from the run queue.
//stratergy: round-robin
extern void schedule();
//Implement in thread.S
//1. Save current regester set into prev->old_reg_set.
//2. Load next->old_reg_set into current register set.
//3. tpidr_el1 := next
extern void switch_to(struct task_reg_set *prev, struct task_reg_set *next);
//return tpidr_el1
extern task_t *get_current_thread();
//responsible for killing zombie threads and try to schedule other runnable thread.
extern void idle_thread();
//call destruct_thread
extern void kill_zombies();
//free freeable resources by it own
//then put itself into stop queue
extern void exit();
extern void thread_demo();
extern void demo_thread();
#endif