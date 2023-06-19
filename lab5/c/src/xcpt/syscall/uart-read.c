#include <stddef.h>

#include "oscos/console.h"
#include "oscos/sched.h"
#include "oscos/uapi/errno.h"
#include "oscos/uapi/unistd.h"
#include "oscos/utils/critical-section.h"

static thread_list_node_t _wait_queue = {.prev = &_wait_queue,
                                         .next = &_wait_queue};

ssize_t sys_uart_read(char buf[const], const size_t size) {
  size_t n_chars_read;

  for (;;) {
    // We must enter critical section here. Otherwise, there will be a race
    // condition between thread suspension and read readiness notification,
    // whose callback adds the current thread to the run queue.

    uint64_t daif_val;
    CRITICAL_SECTION_ENTER(daif_val);

    n_chars_read = console_read_nonblock(buf, size);
    if (n_chars_read != 0) {
      CRITICAL_SECTION_LEAVE(daif_val);
      break;
    }

    console_notify_read_ready((void (*)(void *))add_all_threads_to_run_queue,
                              &_wait_queue);

    suspend_to_wait_queue(&_wait_queue);
    CRITICAL_SECTION_LEAVE(daif_val);
  }

  return n_chars_read;
}
