#include "mem.h"

#define HEAP_SIZE 0x200
// HEAP_SIZE 131072

void* simple_malloc(uint32_t size) {
  static char space[HEAP_SIZE];
  static uint32_t freehead;
  if (freehead + size > HEAP_SIZE) return (char*)0;
  char* target = &space[freehead];
  freehead += size;
  return target;
}