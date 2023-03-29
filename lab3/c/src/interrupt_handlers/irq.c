#include <stdint.h>

#include "oscos/bcm2837/l1ic.h"
#include "oscos/bcm2837/l2ic.h"
#include "oscos/bcm2837/peripheral_memory_barrier.h"
#include "oscos/interrupt_handlers/core_timer.h"
#include "oscos/serial.h"

void irq_handler(void) {
  PERIPHERAL_READ_BARRIER();

  for (;;) {
    const uint32_t int_src = (*CORE_IRQ_SOURCE)[0];
    PERIPHERAL_READ_BARRIER();

    if (int_src == 0)
      break;

    if (int_src & INT_SRC_GPU) {
      const uint32_t l2_int_src = L2IC->irq_pending[0];
      PERIPHERAL_READ_BARRIER();

      if (l2_int_src & INT_SRC_L2_AUX) {
        mini_uart_irq_handler();
      } else {
        serial_fputs("Unknown GPU IRQ source: ");
        serial_print_hex(l2_int_src);
        serial_putc('\n');
      }
    } else if (int_src & INT_SRC_TIMER1) {
      core_timer_interrupt_handler();
    } else {
      serial_fputs("Unknown IRQ source: ");
      serial_print_hex(int_src);
      serial_putc('\n');
    }
  }

  PERIPHERAL_WRITE_BARRIER();
}
