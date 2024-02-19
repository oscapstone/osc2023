#include "thread.h"
#include "my_signal.h"
#include "syscall.h"
#include "page_alloc.h"
#include "stdlib.h"

extern task_struct *get_current();

signal_handler signal_table[] = {
    [0] = &sig_ignore,
    [1] = &sig_ignore,
    [2] = &sig_ignore,
    [3] = &sig_ignore,
    [4] = &sig_ignore,
    [5] = &sig_ignore,
    [6] = &sig_ignore,
    [7] = &sig_ignore,
    [8] = &sig_ignore,
    [SIGKILL] = &sigkill_handler,
};

#define current get_current()

void sig_ignore(int _)
{
    return;
}

void sigkill_handler(int pid)
{
    kill(pid);
    return;
}

void sig_context_update(struct _trapframe *trapframe, void (*handler)())
{
    signal_context *sig_context = (signal_context *)my_malloc(sizeof(signal_context));
    sig_context->trapframe = (struct _trapframe *)my_malloc(sizeof(struct _trapframe));
    sig_context->user_stack = my_malloc(MIN_PAGE_SIZE);
    memcpy(sig_context->trapframe, trapframe, sizeof(struct _trapframe));

    current->signal_context = sig_context;

    trapframe->x[30] = (unsigned long)&sig_return;
    trapframe->elr_el1 = (unsigned long)handler;
    trapframe->sp_el0 = (unsigned long)sig_context->user_stack + MIN_PAGE_SIZE;
}

void sig_return(void)
{
    asm volatile(
        "mov x8, 10\n"
        "svc 0\n");
}

void sig_context_restore(struct _trapframe *trapframe)
{
    memcpy(trapframe, current->signal_context->trapframe, sizeof(struct _trapframe));
}
