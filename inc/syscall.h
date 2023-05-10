#ifndef _SYSCALL_H
#define _SYSCALL_H
#include <trapframe.h>

typedef struct {
    unsigned int iss:25, // Instruction specific syndrome
                 il:1,   // Instruction length bit
                 ec:6;   // Exception class
} esr_el1;

typedef void *(*syscall_funcp)();

void syscall_handler(trapframe regs, uint32 syn);
void *syscall_getpid(void);
void *syscall_uartread(char buf[], size_t size);
void *syscall_uartwrite(const char buf[], size_t size);
void *syscall_exec(const char* name, char *const argv[]);
void *syscall_fork(void);
void *syscall_exit(void);
void *syscall_mbox_call(unsigned char ch, unsigned int *mbox);
void *syscall_kill_pid(int pid);
void *syscall_signal(int signal, void (*handler)(void));
void *syscall_kill(int pid, int signal);
void *syscall_test(void);



#endif