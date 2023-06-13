#ifndef _FORK_H
#define _FORK_H

#include "sched.h"

#define PSR_MODE_EL0t	0x00000000
#define PSR_MODE_EL1t	0x00000004
#define PSR_MODE_EL1h	0x00000005
#define PSR_MODE_EL2t	0x00000008
#define PSR_MODE_EL2h	0x00000009
#define PSR_MODE_EL3t	0x0000000c
#define PSR_MODE_EL3h	0x0000000d

int copy_process(unsigned long, unsigned long, unsigned long, unsigned long);
int move_to_user_mode(unsigned long);
struct pt_regs *task_pt_regs(struct task_struct *);
void new_user_process(unsigned long);

struct pt_regs {
    unsigned long regs[31];     //register
    unsigned long sp;           //stack pointer
    unsigned long pc;           //progeam counter; kernel_exit will copy pc to the elr_el1 register, thus making sure that we will return to the pc address after performing exception return.
    unsigned long pstate;       //process state; This field will be copied to spsr_el1 by the kernel_exit and becomes the processor state after exception return is completed.
};

#endif