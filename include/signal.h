#ifndef _SIGNAL_H
#define _SIGNAL_H
#include "list.h"
#include "stddef.h"
#define USER_SIG_STK_LOW 0xfffffff0b000
#define MAX_SIGNAL 64
#define SIG_KILL 9
typedef void (*signal_handler_t)();
// extern struct trap_frame;
struct signal {
    int signum;
    char *handler_user_stack;
    size_t handler_user_stack_size;
    void *tf;
    int handling;
    list_t node;
};
extern void handle_current_signal();
extern signal_handler_t default_sig_handlers[MAX_SIGNAL+1];
#endif