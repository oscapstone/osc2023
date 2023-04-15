#ifndef _EXCEPTION_H
#define _EXCEPTION_H
#include "utils.h"
#include "stdint.h"
#include "list.h"
#define PRIORITY_BINS 8
extern void disable_local_fiq_interrupt(void);
extern void disable_local_irq_interrupt(void);
extern void disable_local_async_interrupt(void);
extern void disable_local_dbg_interrupt(void);
extern void disable_local_all_interrupt(void);
extern void enable_local_fiq_interrupt(void);
extern void enable_local_irq_interrupt(void);
extern void enable_local_async_interrupt(void);
extern void enable_local_dbg_interrupt(void);
extern void enable_local_all_interrupt(void);

extern int interrupt_cnter;
extern void test_enable_interrupt();
extern void disable_interrupt();

extern void set_exception_vector_table(void);
extern void exception_handler(void);
extern void set_exception_vector_table(void);
extern void default_handler(void);
extern void current_synchronous_exception_router(void);
extern void current_irq_exception_router(void);
extern void lower_synchronous_exception_router(void);
extern void lower_irq_exception_router(void);
extern int get_el(void);
typedef void (*interrupt_handler_t) (void *data);
struct interrupt_task_node {
    interrupt_handler_t handler;
    void *data;
    list_t list;
};
struct interrupt_scheduler {
    //circular linked-lists
    int priority_stack[PRIORITY_BINS];
    int prior_stk_idx;
    list_t qbins[PRIORITY_BINS];
    size_t qsize;
    void (*add_task) (struct interrupt_scheduler *self, interrupt_handler_t handler, void *data, unsigned priority);
};
extern void init_interrupt_scheduler(struct interrupt_scheduler *self);
extern struct interrupt_scheduler _interrupt_scheduler;
#endif