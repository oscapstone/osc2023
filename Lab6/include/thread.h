#ifndef _THREAD_H
#define _THREAD_H

#include "list.h"

#define READY 1
#define ZOMBIE 2
#define FORKING 4

typedef void (*func_ptr)();

typedef struct _context
{
    unsigned long x19, x20, x21, x22, x23, x24, x25, x26, x27, x28, fp, lr, sp;
} context;

typedef struct _thread_info
{
    long id;
    long child_id;
    struct _task_struct *task;

} thread_info;

typedef struct _trapframe
{
    unsigned long x[31];
    unsigned long spsr_el1;
    unsigned long elr_el1;
    unsigned long sp_el0;
} trapframe;

typedef struct _task_struct
{
    struct _context task_context; // context need to be the first one
    struct _thread_info *thread_info;
    struct list_head list;
    func_ptr job;
    unsigned long kstack_start;     // kernel stack base
    unsigned long ustack_start;     // user stack base
    unsigned long usrpgm_load_addr; // user program load address
    unsigned long status;
    unsigned long trapframe;        // using "unsigned long" to keep trapframe address, instead of claiming a "trapframe_t*" to avoid "Data Abort"
} task_struct;

void schedule();
void add_rq(task_struct *task);
task_struct *del_rq();
void thread_init();
thread_info *thread_create(func_ptr fp);
void task_wrapper();
void idle_task();
void kill_zombies();
void do_fork();
void create_child(task_struct *parent);
void debug_task_rq();
void debug_task_zombieq();

#endif /*_THREAD_H */