#include "oscos/fs/vfs.h"
#include "oscos/sched.h"

int sys_mkdir(const char *const pathname, const unsigned mode) {
  (void)mode;

  process_t *const curr_process = current_thread()->process;

  return vfs_mkdir_relative(curr_process->cwd, pathname);
}
