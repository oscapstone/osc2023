#ifndef OSCOS_BCM2837_L1IC_H
#define OSCOS_BCM2837_L1IC_H

#include <stdint.h>

#define CORE_TIMER_IRQCNTL ((volatile uint32_t(*)[4])0x40000040)
#define CORE_IRQ_SOURCE ((volatile uint32_t(*)[4])0x40000060)
#define CORE_FIQ_SOURCE ((volatile uint32_t(*)[4])0x40000070)

#define CORE_TIMER_IRQCNTL_TIMER0_IRQ ((uint32_t)0x01)
#define CORE_TIMER_IRQCNTL_TIMER1_IRQ ((uint32_t)0x02)
#define CORE_TIMER_IRQCNTL_TIMER2_IRQ ((uint32_t)0x04)
#define CORE_TIMER_IRQCNTL_TIMER3_IRQ ((uint32_t)0x08)
#define CORE_TIMER_IRQCNTL_TIMER0_FIQ ((uint32_t)0x10)
#define CORE_TIMER_IRQCNTL_TIMER1_FIQ ((uint32_t)0x20)
#define CORE_TIMER_IRQCNTL_TIMER2_FIQ ((uint32_t)0x40)
#define CORE_TIMER_IRQCNTL_TIMER3_FIQ ((uint32_t)0x80)

#define INT_SRC_TIMER0 ((uint32_t)0x00000001)
#define INT_SRC_TIMER1 ((uint32_t)0x00000002)
#define INT_SRC_TIMER2 ((uint32_t)0x00000004)
#define INT_SRC_TIMER3 ((uint32_t)0x00000008)
#define INT_SRC_MBOX0 ((uint32_t)0x00000010)
#define INT_SRC_MBOX1 ((uint32_t)0x00000020)
#define INT_SRC_MBOX2 ((uint32_t)0x00000040)
#define INT_SRC_MBOX3 ((uint32_t)0x00000080)
#define INT_SRC_GPU ((uint32_t)0x00000100)
#define INT_SRC_PMU ((uint32_t)0x00000200)

#endif
