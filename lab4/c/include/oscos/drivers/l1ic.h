#ifndef OSCOS_DRIVERS_L1IC_H
#define OSCOS_DRIVERS_L1IC_H

#include <stddef.h>
#include <stdint.h>

#define INT_L1_SRC_TIMER0 ((uint32_t)(1 << 0))
#define INT_L1_SRC_TIMER1 ((uint32_t)(1 << 1))
#define INT_L1_SRC_TIMER2 ((uint32_t)(1 << 2))
#define INT_L1_SRC_TIMER3 ((uint32_t)(1 << 3))
#define INT_L1_SRC_MBOX0 ((uint32_t)(1 << 4))
#define INT_L1_SRC_MBOX1 ((uint32_t)(1 << 5))
#define INT_L1_SRC_MBOX2 ((uint32_t)(1 << 6))
#define INT_L1_SRC_MBOX3 ((uint32_t)(1 << 7))
#define INT_L1_SRC_GPU ((uint32_t)(1 << 8))
#define INT_L1_SRC_PMU ((uint32_t)(1 << 9))

void l1ic_init(void);

uint32_t l1ic_get_int_src(size_t core_id);

void l1ic_enable_core_timer_irq(size_t core_id);

#endif
