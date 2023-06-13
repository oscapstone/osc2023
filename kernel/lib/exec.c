#include "cpio.h"
#include "exec.h"
#include "uart.h"
#include "string.h"

void exec(char *program)
{
    char *ustack = smalloc(USTACK_SIZE);

    asm volatile(
        "msr spsr_el1, xzr\n\t"
        "msr elr_el1, %0\n\t"
        "msr sp_el0, %1\n\t"
        "eret\n\t" ::"r"(program),
        "r"(ustack + USTACK_SIZE));
}