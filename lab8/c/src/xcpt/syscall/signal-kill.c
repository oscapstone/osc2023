#include <stdbool.h>

#include "oscos/sched.h"
#include "oscos/uapi/errno.h"

static bool _is_valid_signal_num(const int signal) {
  return 1 <= signal && signal <= 31;
}

int sys_signal_kill(const int pid, const int signal) {
  if (!_is_valid_signal_num(signal))
    return -EINVAL;

  if (pid < 0) {
    deliver_signal_to_all_processes(signal);
  } else {
    process_t *const process = get_process_by_id(pid);
    if (!process)
      return -ESRCH;

    deliver_signal(process, signal);
  }

  return 0;
}
