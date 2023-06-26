#ifndef __SIGNAL_H
#define __SIGNAL_H

#include "ds/list.h"
#include "thread.h"

#define SIGNUMS 10

typedef void (*signal_handler)();

struct Signal_t {
    uint8_t sig_num;
    signal_handler handler;
    void *sig_return;
    struct Thread_t *th;
    struct ds_list_head sig_head;
};

void signal_register(uint8_t signum, signal_handler handler);
void signal_kill(uint32_t pid, uint8_t signum);
void signal_sigreturn();
void signal_handler_exec(uint8_t signum);
void signal_check();


#endif