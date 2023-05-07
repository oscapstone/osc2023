#ifndef SYSCALL_H
#define SYSCALL_H
#include <stddef.h>
#include <stdint.h>
#include "thread.h"

int sys_getpid(void);
size_t sys_uart_read(char*, size_t size);
size_t sys_uart_write(const char*, size_t size);
int sys_exec(const char*, char* const argv[]);
int sys_fork(Trap_frame*);
void sys_exit(int status);
int sys_mbox_call(unsigned char ch, unsigned int *mbox);
void sys_kill(int pid);
uint64_t sys_mmap(uint64_t, size_t len, int prot, int flags, int fd, int);

int get_pid(void);
size_t uart_read(char*, size_t size);
size_t uart_write(const char*, size_t size);
int exec(const char*, char* const argv[]);
int fork();
void uexit();
int mbox_call(unsigned char ch, unsigned int *mbox);
void kill(int pid);
void sys_signal(int, void (*fn)());
void posix_kill(int, int);

// TEST
void fork_test();
void check_timer();

// Container
void handler_container();

#endif //SYSCALL_H
