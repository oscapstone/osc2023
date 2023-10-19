#ifndef OSCOS_USER_PROGRAM_LIBC_UNISTD_H
#define OSCOS_USER_PROGRAM_LIBC_UNISTD_H

#include <stddef.h>

#include "oscos-uapi/unistd.h"

typedef int pid_t;

pid_t getpid(void);

ssize_t uart_read(void *buf, size_t count);
ssize_t uart_write(const void *buf, size_t count);

int exec(const char *pathname, char *const argv[]);

pid_t fork(void);

int close(int fd);

ssize_t write(int fd, const void *buf, size_t count);
ssize_t read(int fd, void *buf, size_t count);

int chdir(const char *path);

long lseek64(int fd, long offset, int whence);

long syscall(long number, ...);

#endif
