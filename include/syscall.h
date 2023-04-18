#ifndef _SYSCALL_H
#define _SYSCALL_H
#include "stdint.h"
#include "thread.h"
typedef void (*syscall_t)(struct trap_frame *);
enum {
    SYS_GETPID,     //0
    SYS_UART_RECV,
    SYS_UART_WRITE,
    SYS_EXEC,
    SYS_FORK,
    SYS_EXIT,       //5
    SYS_MBOX,
    SYS_KILL,
    SYS_SIGNAL,
    SYS_SIGKILL,
    SYS_SIGRETURN,  //10
    NUM_syscalls
};
extern syscall_t default_syscall_table[];
//implement in syscall.S
extern int getpid();
extern size_t uart_read(char buf[], size_t size);
extern size_t uart_write(const char buf[], size_t size);
extern int exec(const char* name, char *const argv[]);
extern int fork();
extern void exit();
extern int mbox_call(unsigned char ch, unsigned int *mbox);
extern void kill(int pid);
extern void fork_test();
extern void run_user_prog(char *user_text);
#endif