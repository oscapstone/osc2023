#include "oscos/sched.h"
#include "oscos/uapi/errno.h"

long sys_lseek64(const int fd, const long offset, const int whence) {
  process_t *const curr_process = current_thread()->process;

  if (!(0 <= fd && fd < N_FDS && curr_process->fds[fd]))
    return -EBADF;

  return vfs_lseek64(curr_process->fds[fd]->file, offset, whence);
}
