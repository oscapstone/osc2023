#include "oscos/fs/vfs.h"
#include "oscos/sched.h"

int sys_chdir(const char *const path) {
  process_t *const curr_process = current_thread()->process;

  struct vnode *next_cwd;
  const int result = vfs_lookup_relative(curr_process->cwd, path, &next_cwd);
  if (result < 0)
    return result;

  curr_process->cwd = next_cwd;
  return 0;
}
