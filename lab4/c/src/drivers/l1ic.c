#include "oscos/drivers/l1ic.h"

#include "oscos/drivers/board.h"

#define CORE_TIMER_IRQCNTL                                                     \
  ((volatile uint32_t(*)[4])((char *)ARM_LOCAL_PERIPHERAL_BASE + 0x40))
#define CORE_IRQ_SOURCE                                                        \
  ((volatile uint32_t(*)[4])((char *)ARM_LOCAL_PERIPHERAL_BASE + 0x60))
#define CORE_FIQ_SOURCE                                                        \
  ((volatile uint32_t(*)[4])((char *)ARM_LOCAL_PERIPHERAL_BASE + 0x70))

#define CORE_TIMER_IRQCNTL_TIMER0_IRQ ((uint32_t)(1 << 0))
#define CORE_TIMER_IRQCNTL_TIMER1_IRQ ((uint32_t)(1 << 1))
#define CORE_TIMER_IRQCNTL_TIMER2_IRQ ((uint32_t)(1 << 2))
#define CORE_TIMER_IRQCNTL_TIMER3_IRQ ((uint32_t)(1 << 3))
#define CORE_TIMER_IRQCNTL_TIMER0_FIQ ((uint32_t)(1 << 4))
#define CORE_TIMER_IRQCNTL_TIMER1_FIQ ((uint32_t)(1 << 5))
#define CORE_TIMER_IRQCNTL_TIMER2_FIQ ((uint32_t)(1 << 6))
#define CORE_TIMER_IRQCNTL_TIMER3_FIQ ((uint32_t)(1 << 7))

void l1ic_init(void) {
  // No-op.
}

uint32_t l1ic_get_int_src(const size_t core_id) {
  const uint32_t result = (*CORE_IRQ_SOURCE)[core_id];

  PERIPHERAL_READ_BARRIER();
  return result;
}

void l1ic_enable_core_timer_irq(size_t core_id) {
  // nCNTPNSIRQ IRQ control = 1
  (*CORE_TIMER_IRQCNTL)[core_id] = CORE_TIMER_IRQCNTL_TIMER1_IRQ;
}
