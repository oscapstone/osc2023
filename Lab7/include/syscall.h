#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "stdlib.h"

int getpid();
size_t uart_read(char buf[], size_t size);
size_t uart_write(const char buf[], size_t size);
int exec(const char *name, char *const argv[]);
int fork();
void exit(int status);
int mbox_call_u(unsigned char ch, unsigned int *mbox);
void kill(int pid);
void signal(int SIGNAL, void (*handler)());

int open(char *pathname, int flags);
int close(int fd);
long write(int fd, void *buf, unsigned long count);
long read(int fd, void *buf, unsigned long count);
int mkdir(char *pathname, unsigned mode);
int mount(char *src, char *target, char *filesystem, unsigned long flags, void *data);
int chdir(char *path);
long lseek(int fd, long offset, int whence);
int ioctl(int fd, unsigned long request, unsigned long arg);

#endif /*_SYSCALL_H */