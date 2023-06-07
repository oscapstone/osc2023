#ifndef _SIGNAL_H
#define _SIGNAL_H

#include "list.h"
#include "thread.h"

#define SIGKILL 9
#define SIG_NUM (sizeof(signal_table) / sizeof(signal_table[0]))
typedef void (*signal_handler)(int);

typedef struct _custom_signal
{
    unsigned int sig_num;
    signal_handler handler;
    struct list_head list;
} custom_signal;

typedef struct _signal_context
{
    struct _trapframe *trapframe;
    char *user_stack;
} signal_context;

void sig_ignore(int _);
void sigkill_handler(int pid);
void sig_context_update(struct _trapframe *trapframe, void (*handler)());
void sig_return(void);
void sig_context_restore(struct _trapframe *trapframe);

#endif /*_SIGNAL_H */
