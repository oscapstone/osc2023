#ifndef IRQ_H
#define IRQ_H


#include "mmu.h"
#include "trapframe.h"

#define CORE0_INTERRUPT_SOURCE ((volatile unsigned int *)(PHYS_TO_VIRT(0x40000060)))

#define PERIPHERAL_INTERRUPT_BASE PHYS_TO_VIRT(0x3F000000)
#define IRQ_BASIC_PENDING ((volatile unsigned int *)(PERIPHERAL_INTERRUPT_BASE + 0x0000B200))

#define INTERRUPT_SOURCE_TIMER (1 << 1)
#define INTERRUPT_SOURCE_CNTPNSIRQ 1<<1
#define INTERRUPT_SOURCE_GPU (1 << 8)
#define IRQ_PENDING_1_AUX_INT (1 << 29)

#define IRQ_PENDING_1 ((volatile unsigned int *)(PERIPHERAL_INTERRUPT_BASE + 0x0000B204))
#define IRQ_PENDING_2 ((volatile unsigned int *)(PERIPHERAL_INTERRUPT_BASE + 0x0000B208))


#define FIQ_CONTROL ((volatile unsigned int *)(PERIPHERAL_INTERRUPT_BASE + 0x0000B20C))
#define ENABLE_IRQS_1 ((volatile unsigned int *)(PERIPHERAL_INTERRUPT_BASE + 0x0000B210))
#define ENABLE_IRQS_2 ((volatile unsigned int *)(PERIPHERAL_INTERRUPT_BASE + 0x0000B214))
#define ENABLE_BASIC_IRQS ((volatile unsigned int *)(PERIPHERAL_INTERRUPT_BASE + 0x0000B218))
#define DISABLE_IRQS_1 ((volatile unsigned int *)(PERIPHERAL_INTERRUPT_BASE + 0x0000B21C))
#define DISABLE_IRQS_2 ((volatile unsigned int *)(PERIPHERAL_INTERRUPT_BASE + 0x0000B220))
#define DISABLE_BASIC_IRQS ((volatile unsigned int *)(PERIPHERAL_INTERRUPT_BASE + 0x0000B224))


 #define DATA_ABORT_LOWER 0b100100
 #define INS_ABORT_LOWER 0b100000

 #define TF_LEVEL0 0b000100
 #define TF_LEVEL1 0b000101
 #define TF_LEVEL2 0b000110
 #define TF_LEVEL3 0b000111





void enable_interrupt();
void disable_interrupt();
void irq_handler();
void invalid_exception_handler(unsigned long long x0);
void cpacr_el1_off();
void sync_el0_64_handler(trapframe_t *tpf, unsigned long x1);

void highp();
void lowp();
void test_preemption();

void lock();
void unlock();
#endif