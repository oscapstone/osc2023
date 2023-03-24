#include "oscos/el.h"

#include <stdint.h>

unsigned get_current_el(void) {
  uint64_t currentel_val;
  __asm__ __volatile__("mrs %0, currentel" : "=r"(currentel_val));
  return (currentel_val >> 2) & 0x3;
}
