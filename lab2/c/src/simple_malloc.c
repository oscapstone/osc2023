#include "oscos/simple_malloc.h"

#include <stdalign.h>
#include <stdint.h>

#include "oscos/align.h"

// Symbol defined in the linker script.
extern char _ekernel[];

#define ALLOC_ALIGN(X) ALIGN(X, alignof(max_align_t))

#define ALLOC_AREA_START ((char *)ALLOC_ALIGN((uintptr_t)_ekernel))
#define ALLOC_AREA_SZ 0x100000 // 1 MB
#define ALLOC_AREA_END (ALLOC_AREA_START + ALLOC_AREA_SZ)

void *simple_malloc(const size_t size) {
  // `alloc_head` should have been initialized to `ALLOC_AREA_START`, but it
  // cannot be done directly. Instead, this variable is initialized lazily on
  // the first invocation of this function.
  static char *alloc_head = NULL;

  if (!alloc_head) { // First invocation; `alloc_head` has not been initialized.
    alloc_head = ALLOC_AREA_START;
  }

  char *const end = alloc_head + size;
  if (end > ALLOC_AREA_END) // Not enough memory.
    return NULL;

  void *const result = alloc_head;
  alloc_head = (char *)ALLOC_ALIGN((uintptr_t)end);
  return result;
}
