#include "oscos/delay.h"

void delay_ns(const uint64_t ns) {
  uint64_t start_counter_val;
  // Normally, we need "isb sy" here to prevent the counter from being read too
  // early due to out-of-order execution. However, since Cortex-A53 issues
  // instructions in order, we don't need it.
  __asm__ __volatile__("mrs %0, CNTPCT_EL0" : "=r"(start_counter_val)::);

  uint64_t counter_clock_frequency_hz;
  __asm__ __volatile__("mrs %0, CNTFRQ_EL0"
                       : "=r"(counter_clock_frequency_hz)::);
  counter_clock_frequency_hz &= 0xffffffff;

  // ceil(ns * counter_clock_frequency_hz / NS_PER_SEC).
  const uint64_t delta_counter_val =
      (ns * counter_clock_frequency_hz + (NS_PER_SEC - 1)) / NS_PER_SEC;

  for (;;) {
    uint64_t counter_val;
    __asm__ __volatile__("mrs %0, CNTPCT_EL0" : "=r"(counter_val)::);

    if (counter_val - start_counter_val >= delta_counter_val)
      break;
  }
}
