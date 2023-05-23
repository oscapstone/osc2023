#include "oscos/mem/malloc.h"

void malloc_init(void) {}

void *malloc(const size_t size) {
  // Stub.
  (void)size;
  return NULL;
}

void free(void *ptr) {
  if (!ptr)
    return;

  // Stub.
}
