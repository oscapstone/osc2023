#include "fcntl.h"

#include "sys/syscall.h"
#include "unistd.h"

int open(const char *const pathname, const int flags) {
  return syscall(SYS_open, pathname, flags);
}
