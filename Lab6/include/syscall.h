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

#endif /*_SYSCALL_H */