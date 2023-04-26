#include "allocator.h"
#include "uart.h"


extern int __bss_end;
static char *head;

int heap_init() {
  head = (char *)&__bss_end;
  head++;
  return 0;
}

void *malloc(int size) {
  void *ptr = (void *)head;
  if (size <= 0)
    return 0;
  head += size;
  return ptr;
}
