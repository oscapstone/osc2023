#include "stdlib.h"

#include "sys/syscall.h"
#include "unistd.h"

void exit(const int status) {
  syscall(SYS_exit, status);
  __builtin_unreachable();
}
