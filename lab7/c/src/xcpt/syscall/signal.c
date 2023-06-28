#include <stdbool.h>

#include "oscos/sched.h"
#include "oscos/uapi/errno.h"

static bool _is_valid_signal_num(const int signal) {
  return 1 <= signal && signal <= 31;
}

sighandler_t sys_signal(const int signal, const sighandler_t handler) {
  if (!_is_valid_signal_num(signal))
    return (sighandler_t)-EINVAL;

  return set_signal_handler(current_thread()->process, signal, handler);
}
