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

struct Trapframe_t{
    uint64_t x0;
    uint64_t x1;
    uint64_t x2;
    uint64_t x3;
    uint64_t x4;
    uint64_t x5;
    uint64_t x6;
    uint64_t x7;
    uint64_t x8;
    uint64_t x9;
    uint64_t x10;
    uint64_t x11;
    uint64_t x12;
    uint64_t x13;
    uint64_t x14;
    uint64_t x15;
    uint64_t x16;
    uint64_t x17;
    uint64_t x18;
    uint64_t x19;
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
    uint64_t x29;
    uint64_t x30;
    uint64_t elr_el1;
    uint64_t sp;
    uint64_t spsr_el1;
    uint64_t esr_el1;
};

void exception_entry();
void enable_interrupt();
void disable_interrupt();
void irqhandler_inc();
void irqhandler_dec();
int irqhandler_cnt_get();
uint64_t interrupt_disable_save();
void interrupt_enable_restore(uint64_t flag);
void lower_irq_handler(struct Trapframe_t *frame);
void spx_irq_handler(struct Trapframe_t *frame, uint64_t esr);
void svc_handler(struct Trapframe_t *frame);
extern struct k_event_queue event_queue;
#endif