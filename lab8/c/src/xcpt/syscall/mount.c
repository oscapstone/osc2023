#include "oscos/fs/vfs.h"
#include "oscos/sched.h"

int sys_mount(const char *const src, const char *const target,
              const char *const filesystem, const unsigned long flags,
              const void *const data) {
  (void)src;
  (void)flags;
  (void)data;

  process_t *const curr_process = current_thread()->process;

  return vfs_mount_relative(curr_process->cwd, target, filesystem);
}
