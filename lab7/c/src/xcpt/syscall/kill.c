#include "oscos/uapi/errno.h"

#include "oscos/sched.h"

int sys_kill(const int pid) {
  if (pid <= 0) {
    kill_all_processes();
  } else {
    process_t *const process = get_process_by_id(pid);
    if (!process)
      return -ESRCH;

    kill_process(process);
  }

  return 0;
}
