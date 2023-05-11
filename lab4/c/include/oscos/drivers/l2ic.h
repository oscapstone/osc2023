#ifndef OSCOS_DRIVERS_L2IC_H
#define OSCOS_DRIVERS_L2IC_H

#include <stdint.h>

#define INT_L2_IRQ_0_SRC_AUX ((uint32_t)(1 << 29))

void l2ic_init(void);

uint32_t l2ic_get_pending_irq_0(void);

void l2ic_enable_irq_0(uint32_t mask);

#endif
