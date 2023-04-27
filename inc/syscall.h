#ifndef _SYSCALL_H
#define _SYSCALL_H

typedef struct {
    unsigned int iss:25, // Instruction specific syndrome
                 il:1,   // Instruction length bit
                 ec:6;   // Exception class
} esr_el1;

typedef void *(*funcp)();

void syscall_handler(uint32 syn);
void *syscall_exit();
void *syscall_test();



#endif