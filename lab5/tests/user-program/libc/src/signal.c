#include "signal.h"

#include "sys/syscall.h"

int kill(const pid_t pid) { return syscall(SYS_kill, pid); }
