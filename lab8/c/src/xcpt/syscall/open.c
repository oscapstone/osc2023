#include "oscos/fs/vfs.h"
#include "oscos/sched.h"
#include "oscos/uapi/errno.h"

int sys_open(const char *const pathname, const int flags) {
  process_t *const curr_process = current_thread()->process;

  // Find the first available FD.

  size_t fd;
  for (fd = 0; fd < N_FDS && curr_process->fds[fd]; fd++)
    ;

  if (fd == N_FDS) // File descriptors used up.
    return -EMFILE;

  // Open the file.

  struct file *file;
  const int result =
      vfs_open_relative(curr_process->cwd, pathname, flags, &file);
  if (result < 0)
    return result;

  shared_file_t *const shared_file = shared_file_new(file);
  if (!shared_file) {
    vfs_close(file);
    return -ENOMEM;
  }

  curr_process->fds[fd] = shared_file;

  return 0;
}
