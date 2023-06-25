#include "oscos/sched.h"
#include "oscos/uapi/errno.h"

int sys_getpid(void) { return current_thread()->process->id; }
