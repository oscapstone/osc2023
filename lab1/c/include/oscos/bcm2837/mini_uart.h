#ifndef OSCOS_BCM2837_MINI_UART_H
#define OSCOS_BCM2837_MINI_UART_H

#include <stdint.h>

#include "oscos/bcm2837/aux.h"

#define AUX_MU_BASE ((void *)((uintptr_t)AUX_BASE + 0x40))

typedef struct {
  volatile uint32_t IO_REG;
  volatile uint32_t IER_REG;
  volatile uint32_t IIR_REG;
  volatile uint32_t LCR_REG;
  volatile uint32_t MCR_REG;
  const volatile uint32_t LSR_REG;
  const volatile uint32_t MSR_REG;
  volatile uint32_t SCRATCH;
  volatile uint32_t CNTL_REG;
  const volatile uint32_t STAT_REG;
  volatile uint32_t BAUD;
} AUX_MU_t;

#define AUX_MU ((AUX_MU_t *)AUX_MU_BASE)

#define AUX_MU_IO_REG_TRANSMIT_DATA_WRITE ((uint32_t)0xff)
#define AUX_MU_IO_REG_RECEIVE_DATA_READ ((uint32_t)0xff)
#define AUX_MU_LSR_REG_DATA_READY ((uint32_t)0x1)
#define AUX_MU_LSR_REG_TRANSMITTER_IDLE ((uint32_t)0x40)
#define AUX_MU_LSR_REG_TRANSMITTER_EMPTY ((uint32_t)0x20)

#endif
