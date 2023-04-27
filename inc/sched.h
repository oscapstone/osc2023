#ifndef _SCHED_H
#define _SCHED_H

#include <type.h>
#include <list.h>

struct pt_regs {
    void *x19;
    void *x20;
    void *x21;
    void *x22;
    void *x23;
    void *x24;
    void *x25;
    void *x26;
    void *x27;
    void *x28;
    void *fp;
    void *lr;
    void *sp;
};

typedef struct _task_struct {
    /* This must be the first element */
    struct pt_regs regs;
    void *kernel_stack;
    struct list_head list;
    uint32 need_resched:1;
    uint32 tid;
} task_struct;

void sched_tick();

void sched_add_task(task_struct *task);

#endif