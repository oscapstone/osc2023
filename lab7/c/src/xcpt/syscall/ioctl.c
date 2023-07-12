#include "oscos/sched.h"
#include "oscos/uapi/errno.h"

int sys_ioctl(const int fd, const unsigned long request, void *const payload) {
  process_t *const curr_process = current_thread()->process;

  if (!(0 <= fd && fd < N_FDS && curr_process->fds[fd]))
    return -EBADF;

  return vfs_ioctl(curr_process->fds[fd]->file, request, payload);
}
