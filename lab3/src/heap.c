#include "heap.h"
#include "uart.h"

/**
 * heap Initializtion
 * Put the heap base to the end of the kernel
 * Using linker symbol _end to get the address.
 */

extern int _end;
/*******************************************
 * Keep track of the heap head
 ******************************************/
static char *head;

/**
 * Avoid override the location of data
 */
int heap_init() {
  head = (char *)&_end;
  head += 4;
  return 0;
}

// Note: Padding
void *malloc(int t) {
  int pad = (4 - (t % 4)) % 4;
  void *tmp = (void *)head;
  if (t <= 0)
    return 0;
  head += t;
  head += pad;
  return tmp;
}
