#ifndef _SYSCALL_H
#define _SYSCALL_H

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#define read_sysreg(r) ({                       \
    uint64 __val;                               \
    asm volatile("mrs %0, " #r : "=r" (__val)); \
    __val;                                      \
})

typedef struct {
    unsigned int iss:25, // Instruction specific syndrome
                 il:1,   // Instruction length bit
                 ec:6;   // Exception class
} esr_el1;

typedef void *(*funcp)();

void syscall_handler(uint32 syn);
void *syscall_exit();
void *syscall_test();

funcp syscall_table[] = {
    syscall_test, // 0
    syscall_exit, // 1
};

#endif