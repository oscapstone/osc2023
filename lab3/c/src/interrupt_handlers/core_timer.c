#include "oscos/interrupt_handlers/core_timer.h"

#include <stdint.h>

#include "oscos/bcm2837/l1ic.h"
#include "oscos/serial.h"

void core_timer_interrupt_enable(void) {
  uint64_t core_timer_freq;
  __asm__ __volatile__("mrs %0, cntfrq_el0" : "=r"(core_timer_freq));
  core_timer_freq &= 0xffffffff;

  // Enable the core timer interrupt. (ENABLE = 1, IMASK = 0)
  __asm__ __volatile__("msr cntp_ctl_el0, %0" : : "r"(0x1));

  // Set the time to interrupt to 2 seconds later.
  __asm__ __volatile__("msr cntp_tval_el0, %0" : : "r"(2 * core_timer_freq));

  // Enable the core 0 timer interrupt at the L1 interrupt controller.
  // (nCNTPNSIRQ IRQ control = 1)
  (*CORE_TIMER_IRQCNTL)[0] = CORE_TIMER_IRQCNTL_TIMER1_IRQ;
}

void core_timer_interrupt_handler(void) {
  uint64_t core_timer_freq;
  __asm__ __volatile__("mrs %0, cntfrq_el0" : "=r"(core_timer_freq));
  core_timer_freq &= 0xffffffff;

  // Print the number of seconds since boot.

  uint64_t timer_val;
  __asm__ __volatile__("mrs %0, cntpct_el0" : "=r"(timer_val));

  serial_fputs("# seconds since boot: 0x");
  serial_print_hex_u64(timer_val / core_timer_freq);
  serial_putc('\n');

  // Set the time to interrupt to 2 seconds later.

  __asm__ __volatile__("msr cntp_tval_el0, %0" : : "r"(2 * core_timer_freq));
}
