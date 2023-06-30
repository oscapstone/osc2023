#include <stddef.h>

#include "oscos/console.h"
#include "oscos/sched.h"
#include "oscos/uapi/errno.h"
#include "oscos/utils/critical-section.h"

static thread_list_node_t _wait_queue = {.prev = &_wait_queue,
                                         .next = &_wait_queue};

size_t sys_uart_write(const char buf[const], const size_t size) {
  size_t n_chars_written;

  for (;;) {
    // We must enter critical section here. Otherwise, there will be a race
    // condition between thread suspension and write readiness notification,
    // whose callback adds the current thread to the run queue.

    uint64_t daif_val;
    CRITICAL_SECTION_ENTER(daif_val);

    n_chars_written = console_write_nonblock(buf, size);
    if (n_chars_written != 0) {
      CRITICAL_SECTION_LEAVE(daif_val);
      break;
    }

    thread_t *const curr_thread = current_thread();

    console_notify_write_ready(
        (void (*)(void *))wake_up_all_threads_in_wait_queue, &_wait_queue);
    suspend_to_wait_queue(&_wait_queue);
    XCPT_MASK_ALL();

    if (curr_thread->status.is_waken_up_by_signal) {
      curr_thread->status.is_waken_up_by_signal = false;
      CRITICAL_SECTION_LEAVE(daif_val);
      return -EINTR;
    }

    CRITICAL_SECTION_LEAVE(daif_val);
  }

  return n_chars_written;
}
