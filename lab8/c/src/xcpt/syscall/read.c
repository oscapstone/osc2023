#include "oscos/fs/vfs.h"
#include "oscos/sched.h"
#include "oscos/uapi/errno.h"

long sys_read(const int fd, void *const buf, const unsigned long count) {
  process_t *const curr_process = current_thread()->process;

  if (!(0 <= fd && fd < N_FDS && curr_process->fds[fd]))
    return -EBADF;

  return vfs_read(curr_process->fds[fd]->file, buf, count);
}
