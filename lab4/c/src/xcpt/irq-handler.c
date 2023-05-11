#include <stdint.h>

#include "oscos/console.h"
#include "oscos/drivers/board.h"
#include "oscos/drivers/l1ic.h"
#include "oscos/drivers/l2ic.h"
#include "oscos/libc/inttypes.h"
#include "oscos/timer/timeout.h"

void xcpt_irq_handler(void) {
  PERIPHERAL_READ_BARRIER();

  uint64_t core_id;
  __asm__ __volatile__("mrs %0, mpidr_el1" : "=r"(core_id));
  core_id &= 0x3;

  for (;;) {
    const uint32_t int_src = l1ic_get_int_src(core_id);

    if (int_src == 0)
      break;

    if (int_src & INT_L1_SRC_GPU) {
      const uint32_t l2_int_src = l2ic_get_pending_irq_0();

      if (l2_int_src & INT_L2_IRQ_0_SRC_AUX) {
        mini_uart_interrupt_handler();
      } else {
        console_printf(
            "WARN: Received an IRQ from GPU with an unknown source: %" PRIx32
            "\n",
            l2_int_src);
      }
    } else if (int_src & INT_L1_SRC_TIMER1) {
      xcpt_core_timer_interrupt_handler();
    } else {
      console_printf("WARN: Received an IRQ with an unknown source: %" PRIx32
                     "\n",
                     int_src);
    }
  }

  PERIPHERAL_WRITE_BARRIER();
}
