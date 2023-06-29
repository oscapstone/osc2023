#ifndef OSCOS_USER_PROGRAM_LIBC_SYS_MMAN_H
#define OSCOS_USER_PROGRAM_LIBC_SYS_MMAN_H

#include <stddef.h>

#include "oscos-uapi/sys/mman.h"

typedef int off_t;

void *mmap(void *addr, size_t length, int prot, int flags, int fd,
           off_t offset);

#endif
