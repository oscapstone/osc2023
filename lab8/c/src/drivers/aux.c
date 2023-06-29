#include "oscos/drivers/aux.h"

#include <stdint.h>

#include "oscos/drivers/board.h"

#define AUX_REG_BASE ((void *)((char *)PERIPHERAL_BASE + 0x215000))

typedef struct {
  const volatile uint32_t irq;
  volatile uint32_t enb;
} aux_reg_t;

#define AUX_REG ((aux_reg_t *)AUX_REG_BASE)

#define AUX_ENB_MINI_UART_ENABLE ((uint32_t)(1 << 0))

void aux_init(void) {
  // No-op.
}

void aux_enable_mini_uart(void) {
  PERIPHERAL_WRITE_BARRIER();

  AUX_REG->enb |= AUX_ENB_MINI_UART_ENABLE;

  PERIPHERAL_READ_BARRIER();
}
