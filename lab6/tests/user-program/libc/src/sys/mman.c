#include "sys/mman.h"

#include "sys/syscall.h"
#include "unistd.h"

void *mmap(void *const addr, const size_t length, const int prot,
           const int flags, const int fd, const off_t offset) {
  return (void *)syscall(SYS_mmap, addr, length, prot, flags, fd, offset);
}
