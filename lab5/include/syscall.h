#ifndef SYSCALL_H
#define SYSCALL_H
#include <stddef.h>
#include "thread.h"

int sys_getpid(void);
size_t sys_uart_read(char*, size_t size);
size_t sys_uart_write(const char*, size_t size);
int sys_exec(const char*, char* const argv[]);
int sys_fork(Trap_frame*);
void sys_exit(int status);
int sys_mbox_call(unsigned char ch, unsigned int *mbox);
void sys_kill(int pid);

int get_pid(void);
size_t uart_read(char*, size_t size);
size_t uart_write(const char*, size_t size);
int exec(const char*, char* const argv[]);
int fork();
void uexit();
int mbox_call(unsigned char ch, unsigned int *mbox);
void kill(int pid);

// TEST
void fork_test();
void check_timer();

#endif //SYSCALL_H
