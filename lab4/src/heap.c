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
  // head = 0x1000000;
  int pad = (8 - ((int)&_end % 8)) % 8;
  head += pad;
  head += 8;
  return 0;
}

// Note: Padding
void *malloc(int t) {
  int pad = (8 - (t % 8)) % 8;
  void *tmp = (void *)head;
  if (t <= 0)
    return 0;
  head += t;
  head += pad;
  return tmp;
}
