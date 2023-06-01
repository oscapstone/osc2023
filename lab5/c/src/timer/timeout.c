#include "oscos/timer/timeout.h"

#include <stddef.h>

#include "oscos/drivers/l1ic.h"
#include "oscos/utils/core-id.h"
#include "oscos/utils/critical-section.h"
#include "oscos/utils/heapq.h"
#include "oscos/utils/time.h"
#include "oscos/xcpt.h"

#define MAX_N_TIMEOUT_ENTRIES 16

typedef struct {
  uint64_t timestamp;
  void (*callback)(void *);
  void *arg;
} timeout_entry_t;

static timeout_entry_t _timeout_entries[MAX_N_TIMEOUT_ENTRIES];
static size_t _n_timeout_entries = 0;

static int _timeout_entry_cmp_by_timestamp(const timeout_entry_t *const e1,
                                           const timeout_entry_t *const e2,
                                           void *const _arg) {
  (void)_arg;

  if (e1->timestamp < e2->timestamp)
    return -1;
  if (e1->timestamp > e2->timestamp)
    return 1;
  return 0;
}

void timeout_init(void) {
  __asm__ __volatile__("msr cntp_ctl_el0, %0" : : "r"(0x3));
  l1ic_enable_core_timer_irq(get_core_id());
}

bool timeout_add_timer(void (*const callback)(void *), void *const arg,
                       const uint64_t after_ns) {
  if (_n_timeout_entries == MAX_N_TIMEOUT_ENTRIES)
    return false;

  uint64_t core_timer_freq_hz;
  __asm__("mrs %0, cntfrq_el0" : "=r"(core_timer_freq_hz));
  core_timer_freq_hz &= 0xffffffff;

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  uint64_t curr_timestamp;
  __asm__ __volatile__("mrs %0, cntpct_el0" : "=r"(curr_timestamp));

  // ceil(after_ns * core_timer_freq_hz / NS_PER_SEC).
  const uint64_t delta_timestamp =
      (after_ns * core_timer_freq_hz + (NS_PER_SEC - 1)) / NS_PER_SEC;
  const uint64_t timestamp = curr_timestamp + delta_timestamp;

  // Reprogram the timer.

  const bool need_timer_reprogramming =
      _n_timeout_entries == 0 || timestamp < _timeout_entries[0].timestamp;
  if (need_timer_reprogramming) {
    __asm__ __volatile__("msr cntp_cval_el0, %0" : : "r"(timestamp));
  }

  // Insert the new timeout entry into the timeout queue.

  timeout_entry_t entry = {
      .timestamp = timestamp, .callback = callback, .arg = arg};
  heappush(_timeout_entries, _n_timeout_entries++, sizeof(timeout_entry_t),
           &entry,
           (int (*)(const void *, const void *,
                    void *))_timeout_entry_cmp_by_timestamp,
           NULL);

  // Enable the core timer interrupt. (ENABLE = 1, IMASK = 0)
  __asm__ __volatile__("msr cntp_ctl_el0, %0" : : "r"(0x1));

  CRITICAL_SECTION_LEAVE(daif_val);

  return true;
}

void xcpt_core_timer_interrupt_handler(void) {
  // Remove the top entry from the timeout queue.

  timeout_entry_t top_entry;
  heappop(_timeout_entries, _n_timeout_entries--, sizeof(timeout_entry_t),
          &top_entry,
          (int (*)(const void *, const void *,
                   void *))_timeout_entry_cmp_by_timestamp,
          NULL);

  // Execute the callback.
  top_entry.callback(top_entry.arg);

  // Reprogram the timer.
  if (_n_timeout_entries == 0) {
    // Disable the core timer interrupt. (ENABLE = 1, IMASK = 1)
    __asm__ __volatile__("msr cntp_ctl_el0, %0" : : "r"(0x3));
  } else {
    __asm__ __volatile__("msr cntp_cval_el0, %0"
                         :
                         : "r"(_timeout_entries[0].timestamp));
  }
}
