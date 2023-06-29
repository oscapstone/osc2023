#include "signal.h"

#include "sys/syscall.h"

int kill(const pid_t pid) { return syscall(SYS_kill, pid); }

sighandler_t signal(const int signal, const sighandler_t handler) {
  return (sighandler_t)syscall(SYS_signal, signal, handler);
}

int signal_kill(const pid_t pid, const int signal) {
  return syscall(SYS_signal_kill, pid, signal);
}
