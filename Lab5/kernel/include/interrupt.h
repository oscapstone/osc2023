#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "gpio.h"

#define IRQ_PENDING_1 ((volatile unsigned int *)(MMIO_BASE + 0x0000b204))
#define IRQ_PENDING_1_AUX_INT (1 << 29)

#define CORE0_INT_SRC ((volatile unsigned int *)(0x40000060))
#define CORE0_INT_SRC_GPU (1 << 8)
#define CORE0_INT_SRC_TIMER (1 << 1)

#define UART_IRQ_PRIORITY 5
#define TIMER_IRQ_PRIORITY 10

typedef struct trapframe {
    unsigned long x0;
    unsigned long x1;
    unsigned long x2;
    unsigned long x3;
    unsigned long x4;
    unsigned long x5;
    unsigned long x6;
    unsigned long x7;
    unsigned long x8;
    unsigned long x9;
    unsigned long x10;
    unsigned long x11;
    unsigned long x12;
    unsigned long x13;
    unsigned long x14;
    unsigned long x15;
    unsigned long x16;
    unsigned long x17;
    unsigned long x18;
    unsigned long x19;
    unsigned long x20;
    unsigned long x21;
    unsigned long x22;
    unsigned long x23;
    unsigned long x24;
    unsigned long x25;
    unsigned long x26;
    unsigned long x27;
    unsigned long x28;
    unsigned long x29;
    unsigned long x30;
    unsigned long spsr_el1;
    unsigned long elr_el1;
    unsigned long sp_el0;
} trapframe_t;

void sync_el0_64_handler(trapframe_t * tpf);
void enable_interrupt();
void disable_interrupt();
void irq_handler(unsigned long long x0);
void invalid_exception_handler(unsigned long long x0);
void set_cpacr_el1();

void enter_critical();
void exit_critical();

#endif