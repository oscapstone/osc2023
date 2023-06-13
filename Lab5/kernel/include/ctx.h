#ifndef CXT_H
#define CXT_H

#include "sched.h"

static inline thread_context_t *get_current_ctx(void)
{
    thread_context_t *cur;
    asm volatile(
        "mrs %0, tpidr_el1\n\t"
        : "=r"(cur));
    return cur;
}

static inline void set_current_ctx(thread_context_t *thread_context)
{
    asm volatile(
        "msr tpidr_el1, %0\n\t" 
        ::"r"(thread_context));
}

#define current_ctx get_current_ctx()

#endif