#ifndef _TASK_H
#define _TASK_H

#include <type.h>
#include <sched.h>
#include <list.h>
#include <mmu.h>

#define STACK_SIZE ( 4 * PAGE_SIZE)

/* Task status */

#define TASK_NEW 0
#define TASK_RUNNING 1
#define TASK_DEAD 2

#define SAVE_REGS(task) \
    asm volatile (                          \
        "stp x19, x20, [%x0, 16 * 0]\n"     \
        "stp x21, x22, [%x0, 16 * 1]\n"     \
        "stp x23, x24, [%x0, 16 * 2]\n"     \
        "stp x25, x26, [%x0, 16 * 3]\n"     \
        "stp x27, x28, [%x0, 16 * 4]\n"     \
        "stp fp, lr, [%x0, 16 * 5]\n"       \
        "mov x9, sp\n"                      \
        "str x9, [%x0, 16 * 6]\n"           \
        : : "r" (&task->regs)               \
    );

void task_init(void);

task_struct *task_create(void);
void task_free(task_struct *task);
task_struct *task_get_by_tid(uint32 tid);

/*
 * Create initial mapping for user program
 *
 * 0x00003c000000 ~ 0x00003f000000: rw-: Mailbox address
 * 0x7f0000000000 ~   <shared_len>: r-x: Kernel functions exposed to users
 * 0xffffffffb000 ~   <STACK_SIZE>: rw-: Stack
 */
void task_init_map(task_struct *task);

#endif