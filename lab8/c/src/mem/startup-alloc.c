#include "oscos/mem/startup-alloc.h"

#include <stdint.h>

#include "oscos/utils/align.h"

// Symbol defined in the linker script.
extern char _sheap[];

static char *_next = _sheap;

void startup_alloc_init(void) {
  // No-op.
}

void *startup_alloc(const size_t size) {
  char *const result = (char *)ALIGN((uintptr_t)_next, alignof(max_align_t));
  _next = result + size;
  return result;
}

va_range_t startup_alloc_get_alloc_range(void) {
  return (va_range_t){.start = _sheap, .end = _next};
}
