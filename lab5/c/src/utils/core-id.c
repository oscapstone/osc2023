#include "oscos/utils/core-id.h"

#include <stdint.h>

size_t get_core_id(void) {
  uint64_t mpidr_val;
  __asm__ __volatile__("mrs %0, mpidr_el1" : "=r"(mpidr_val));
  return mpidr_val & 0x3;
}
