#ifndef SIGNAL_H
#define SIGNAL_H

#define SIGKILL_NO 9

#include "syscall.h"
#include "sched.h"
#include "mm.h"

void signal_default_handler();
void check_signal();
void run_signal(int signal);
void signal_handler_wrapper();

#endif