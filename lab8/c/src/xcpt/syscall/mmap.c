#include <stddef.h>

#include "oscos/mem/page-alloc.h"
#include "oscos/mem/vm.h"
#include "oscos/sched.h"
#include "oscos/uapi/errno.h"
#include "oscos/utils/align.h"

void *sys_mmap(void *const addr, const size_t len, const int prot,
               const int flags, const int fd, const int file_offset) {
  (void)flags;
  (void)fd;
  (void)file_offset;

  process_t *const curr_process = current_thread()->process;

  void *const mmap_addr =
      vm_decide_mmap_addr(curr_process->addr_space, addr, len);
  if (!mmap_addr) {
    return (void *)-EINVAL;
  }

  const mem_region_t mem_region = {.start = mmap_addr,
                                   .len = ALIGN(len, 1 << PAGE_ORDER),
                                   .type = MEM_REGION_ANONYMOUS,
                                   .prot = prot};
  vm_mem_regions_insert_region(&curr_process->addr_space.mem_regions,
                               &mem_region);

  return mmap_addr;
}
