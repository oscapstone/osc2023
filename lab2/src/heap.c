#include "heap.h"
#include "uart.h"

/**
 * heap Initializtion
 * Put the heap base to the end of the kernel
 * Using linker symbol _end to get the address.
 */

extern int _end;
static char *head;

/**
 * Avoid override the location of data
 */
int heap_init() {
  head = (char *)&_end;
  head++;
  return 0;
}

void *malloc(int t) {
  void *tmp = (void *)head;
  if (t <= 0)
    return 0;
  head += t;
  return tmp;
}
