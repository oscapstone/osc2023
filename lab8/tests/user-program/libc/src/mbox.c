#include "mbox.h"

#include "sys/syscall.h"
#include "unistd.h"

int mbox_call(const unsigned char ch, unsigned int *const mbox) {
  return syscall(SYS_mbox_call, ch, mbox);
}
