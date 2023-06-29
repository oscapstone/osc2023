#include "sys/mount.h"

#include "sys/syscall.h"
#include "unistd.h"

int mount(const char *const source, const char *const target,
          const char *const filesystemtype, const unsigned long mountflags,
          const void *const data) {
  return syscall(SYS_mount, source, target, filesystemtype, mountflags, data);
}
