#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include "peripherals/mailbox.h"

#define IRQ_PENDING_1           ((volatile unsigned int*)(MMIO_BASE + 0xB204))
#define ENABLE_IRQS_1           ((volatile unsigned int*)(MMIO_BASE + 0xB210))
#define DISABLE_IRQS_1          ((volatile unsigned int*)(MMIO_BASE + 0xB21C))

#define CORE0_TIMER_IRQ_CTRL    ((volatile unsigned int*)(0x40000040))
#define CORE0_IRQ_SOURCE        ((volatile unsigned int*)(0x40000060))

#define IRQ_SOURCE_CNTPNSIRQ    (1 << 1)
#define IRQ_SOURCE_GPU          (1 << 8)
#define IRQ_PENDING_1_AUX_INT   (1 << 29)

void enable_interrupt(void);
void disable_interrupt(void);
void el0_irq_entry(void);
void el1h_irq_entry(void);
void exception_entry(void);
void invalid_exception_entry(void);

#endif