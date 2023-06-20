#ifndef _SYSCALL_H
#define _SYSCALL_H
#include <trapframe.h>

#define KSTACK_VARIABLE(x)                      \
    (void *)((uint64)x -                        \
             (uint64)current->kernel_stack +    \
             (uint64)child->kernel_stack)

#define USTACK_VARIABLE(x)                      \
    (void *)((uint64)x -                        \
             (uint64)current->user_stack +      \
             (uint64)child->user_stack)

#define DATA_VARIABLE(x)                        \
    (void *)((uint64)x -                        \
             (uint64)current->data +            \
             (uint64)child->data)

typedef void *(*syscall_funcp)();

void syscall_handler(trapframe *regs);

void syscall_getpid(trapframe *frame);
void syscall_uart_read(trapframe *_, char buf[], size_t size);
void syscall_uart_write(trapframe *_, const char buf[], size_t size);
void syscall_exec(trapframe *_, const char* name, char *const argv[]);
void syscall_fork(trapframe *frame);
void syscall_exit(trapframe *_);
void syscall_mbox_call(trapframe *_, unsigned char ch, unsigned int *mbox);
void syscall_kill_pid(trapframe *_, int pid);

void syscall_test(trapframe *_);

void exit_to_user_mode(trapframe regs);

#endif