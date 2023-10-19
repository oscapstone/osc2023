#include "oscos/uapi/errno.h"

#include "oscos/sched.h"

int sys_fork_impl(const extended_trap_frame_t *const trap_frame) {
  process_t *const new_process = fork(trap_frame);
  if (!new_process)
    return -ENOMEM;

  return new_process->id;
}
