#include "oscos/drivers/l2ic.h"

#include "oscos/drivers/board.h"

#define L2IC_REG_BASE ((void *)((char *)PERIPHERAL_BASE + 0xb200))

typedef struct {
  const volatile uint32_t irq_basic_pending;
  const volatile uint32_t irq_pending[2];
  volatile uint32_t fiq_control;
  volatile uint32_t enable_irqs[2];
  volatile uint32_t enable_basic_irqs;
  volatile uint32_t disable_irqs[2];
  volatile uint32_t disable_basic_irqs;
} l2ic_reg_t;

#define L2IC_REG ((l2ic_reg_t *)L2IC_REG_BASE)

void l2ic_init(void) {
  // No-op.
}

uint32_t l2ic_get_pending_irq_0(void) {
  const uint32_t result = L2IC_REG->irq_pending[0];

  PERIPHERAL_READ_BARRIER();
  return result;
}

void l2ic_enable_irq_0(uint32_t mask) {
  PERIPHERAL_WRITE_BARRIER();

  L2IC_REG->enable_irqs[0] |= mask;

  PERIPHERAL_READ_BARRIER();
}
