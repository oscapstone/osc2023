#include "oscos/sched.h"
#include "oscos/xcpt.h"

void sys_sigreturn_check(void) {
  XCPT_MASK_ALL();

  // Crash the process if it incorrectly calls sys_sigreturn when not handling
  // signals.
  if (!current_thread()->status.is_handling_signal)
    thread_exit();

  XCPT_UNMASK_ALL();
}
