#ifndef OSCOS_BCM2837_L2IC_H
#define OSCOS_BCM2837_L2IC_H

#include <stdint.h>

#include "oscos/bcm2837/peripheral.h"

#define L2IC_BASE ((void *)((uintptr_t)PERIPHERAL_BASE + 0xb000))

typedef struct {
  const volatile uint32_t _reserved[128];
  const volatile uint32_t irq_basic_pending;
  const volatile uint32_t irq_pending[2];
  volatile uint32_t fiq_control;
  volatile uint32_t enable_irqs[2];
  volatile uint32_t enable_basic_irqs;
  volatile uint32_t disable_irqs[2];
  volatile uint32_t disable_basic_irqs;
} L2IC_t;

#define L2IC ((L2IC_t *)L2IC_BASE)

#define INT_SRC_L2_AUX ((uint32_t)1 << 29)

#endif
