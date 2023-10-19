#include "oscos/sched.h"

#include <limits.h>

#include "oscos/timer/timeout.h"
#include "oscos/xcpt.h"
#include "oscos/xcpt/task-queue.h"

static void _periodic_sched(void *const _arg) {
  (void)_arg;

  sched_setup_periodic_scheduling();

  // Save spsr_el1 and elr_el1, since they can be clobbered by other threads.

  uint64_t spsr_val, elr_val;
  __asm__ __volatile__("mrs %0, spsr_el1" : "=r"(spsr_val));
  __asm__ __volatile__("mrs %0, elr_el1" : "=r"(elr_val));

  schedule();
  XCPT_MASK_ALL();

  __asm__ __volatile__("msr spsr_el1, %0" : : "r"(spsr_val));
  __asm__ __volatile__("msr elr_el1, %0" : : "r"(elr_val));
}

void sched_setup_periodic_scheduling(void) {
  uint64_t core_timer_freq_hz;
  __asm__("mrs %0, cntfrq_el0" : "=r"(core_timer_freq_hz));
  core_timer_freq_hz &= 0xffffffff;

  timeout_add_timer_ticks(_periodic_sched, NULL, core_timer_freq_hz >> 5);
}
