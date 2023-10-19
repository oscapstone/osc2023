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

int close(int fd) { return syscall(SYS_close, fd); }

ssize_t write(const int fd, const void *const buf, const size_t count) {
  return syscall(SYS_write, fd, buf, count);
}

ssize_t read(const int fd, void *const buf, const size_t count) {
  return syscall(SYS_read, fd, buf, count);
}

int chdir(const char *const path) { return syscall(SYS_chdir, path); }

long lseek64(const int fd, const long offset, const int whence) {
  return syscall(SYS_lseek64, fd, offset, whence);
}

void sync(void) { syscall(SYS_sync); }
