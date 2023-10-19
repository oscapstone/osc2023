#include "unistd.h"

#include "sys/syscall.h"

pid_t getpid(void) { return syscall(SYS_getpid); }

ssize_t uart_read(void *const buf, const size_t count) {
  return syscall(SYS_uart_read, buf, count);
}

ssize_t uart_write(const void *const buf, const size_t count) {
  return syscall(SYS_uart_write, buf, count);
}

int exec(const char *const pathname, char *const argv[const]) {
  return syscall(SYS_exec, pathname, argv);
}

pid_t fork(void) { return syscall(SYS_fork); }
