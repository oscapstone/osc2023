#include "malloc.h"

#define MAX_HEAP_SIZE 0x200

extern char __heap_start;
static char *heap_ptr = &__heap_start;
static uint64_t heap_size = MAX_HEAP_SIZE;

void* simple_malloc(uint64_t size) {
  char *ret_ptr = heap_ptr;

  if (size > MAX_HEAP_SIZE) {
    return (void *)0;
  }

  heap_ptr += size;
  heap_size -= size;
  return ret_ptr;
}
