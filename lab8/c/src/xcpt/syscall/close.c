#include "oscos/fs/vfs.h"
#include "oscos/sched.h"
#include "oscos/uapi/errno.h"

int sys_close(const int fd) {
  process_t *const curr_process = current_thread()->process;

  if (!(0 <= fd && fd < N_FDS && curr_process->fds[fd]))
    return -EBADF;

  shared_file_drop(curr_process->fds[fd]);
  curr_process->fds[fd] = NULL;

  return 0;
}
