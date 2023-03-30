#ifndef __EXCEPTION_H
#define __EXCEPTION_H
#include "type.h"
#include "peripherals/base.h"
#include "event.h"

#define IRQ_BASIC_PENDING (PBASE + 0xB200)
#define IRQ_PENDING1 (PBASE + 0xB204)
#define IRQ_PENDING2 (PBASE + 0xB208)
#define ENABLE_IRQ1 (PBASE + 0xb210)
#define ENABLE_IRQ2 (PBASE + 0xb214)
#define DISABLE_IRQ1		(PBASE+0x0000B21C)
#define DISABLE_IRQ2		(PBASE+0x0000B220)

void exception_entry();
void enable_interrupt();
void disable_interrupt();
void irqhandler_inc();
void irqhandler_dec();
int irqhandler_cnt_get();
uint64_t interrupt_disable_save();
void interrupt_enable_restore(uint64_t flag);
extern struct k_event_queue event_queue;
#endif