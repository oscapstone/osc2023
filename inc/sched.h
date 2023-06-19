#ifndef _SCHED_H
#define _SCHED_H

#include <type.h>
#include <list.h>
#include <mmu.h>

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

struct signal_head_t;
struct sighand_t;
typedef struct _task_struct {
    struct pt_regs regs;
    pd_t *page_table;
    /* The order of the above elements cannot be changed*/
    void *kernel_stack;
    void *user_stack;
    /* TODO: Update to address_space*/
    void *data;
    uint32 datalen;
    struct list_head list;
    struct list_head task_list;
    uint16 status;
    uint16 need_resched:1;
    uint32 tid;
    uint32 preempt;
    /* Signal */
    struct signal_head_t *signal;
    struct sighand_t *sighand;
} task_struct;

void switch_to(task_struct *from, task_struct *to);

void scheduler_init();

void sched_tick();

void sched_add_task(task_struct *task);

void schedule();

void sched_del_task(task_struct *task);

#endif