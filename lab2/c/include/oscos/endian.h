#ifndef OSCOS_ENDIAN_H
#define OSCOS_ENDIAN_H

#include <stdalign.h>
#include <stdint.h>

/// \brief Reverses the byte order in a `uint32_t`.
static inline uint32_t rev_u32(const uint32_t x) {
  uint32_t result;
  __asm__("rev %w1, %w0" : "=r"(result) : "r"(x));
  return result;
}

static inline uint32_t load_be_u32_aligned(const char *const p) {
  if (((uintptr_t)p & ~(alignof(uint32_t) - 1)) != 0) {
    __builtin_unreachable();
  }

  const unsigned char *const d = (const unsigned char *)p;
  return (((uint32_t)d[0] << 8 | d[1]) << 8 | d[2]) << 8 | d[3];
}

#endif
