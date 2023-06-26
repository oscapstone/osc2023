#ifndef __INTERRUPT__
#define __INTERRUPT__

#include <stdint.h>
#include <utils.h>

#define MMIO_BASE       0x3f000000
#define ARMINTERRUPT_BASE MMIO_BASE + 0xB000
#define ARMINT_IRQ_PEND_BASE_REG ARMINTERRUPT_BASE + 0x200
#define ARMINT_IRQ_PEND1_REG ARMINTERRUPT_BASE + 0x204
#define ARMINT_IRQ_PEND2_REG ARMINTERRUPT_BASE + 0x208
#define ARMINT_En_IRQs1_REG ARMINTERRUPT_BASE + 0x210
#define CORE0_INTERRUPT_SOURCE 0x40000060

void interrupt_enable();
void interrupt_disable();
void interrupt_irq_handler();
#endif