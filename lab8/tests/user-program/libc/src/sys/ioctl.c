#include "sys/ioctl.h"

#include <stdarg.h>

#include "sys/syscall.h"
#include "unistd.h"

int ioctl(const int fd, const unsigned long request, ...) {
  va_list ap;
  va_start(ap, request);

  void *const payload = va_arg(ap, void *);

  va_end(ap);

  return syscall(SYS_ioctl, fd, request, payload);
}
