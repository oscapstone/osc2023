#include <stdint.h>

#include "oscos/bcm2837/l1ic.h"
#include "oscos/interrupt_handlers/core_timer.h"
#include "oscos/serial.h"

void irq_handler(void) {
  const uint32_t int_src = (*CORE_IRQ_SOURCE)[0];

  if (int_src & INT_SRC_TIMER1) {
    core_timer_interrupt_handler();
  } else {
    serial_fputs("Unknown IRQ source: ");
    serial_print_hex(int_src);
    serial_putc('\n');
  }
}
