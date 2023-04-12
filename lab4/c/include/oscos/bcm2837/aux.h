#ifndef OSCOS_BCM2837_AUX_H
#define OSCOS_BCM2837_AUX_H

#include <stdint.h>

#include "oscos/bcm2837/peripheral.h"

#define AUX_BASE ((void *)((uintptr_t)PERIPHERAL_BASE + 0x215000))

typedef struct {
  const volatile uint32_t IRQ;
  volatile uint32_t ENB;
} AUX_t;

#define AUX ((AUX_t *)AUX_BASE)

#define AUXENB_MINI_UART_ENABLE ((uint32_t)0x1)

#endif
