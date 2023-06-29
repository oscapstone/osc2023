#ifndef OSCOS_UTILS_MATH_H
#define OSCOS_UTILS_MATH_H

#include <stdint.h>

/// \brief Returns the ceiling of the log base 2 of the argument.
static inline uint64_t clog2(const uint64_t x) {
  uint64_t clz_result;
  __asm__("clz %0, %1" : "=r"(clz_result) : "r"(x - 1));
  return 64 - clz_result;
}

#endif
