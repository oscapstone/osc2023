#include "malloc.h"
extern char _heap_start;
static char* top = &_heap_start;

// like C malloc
void* malloc(unsigned int size) {
    char* r = top+0x10;
    if(size<0x18)size=0x18;  // minimum size 0x20 //like ptmalloc
    size = size + 0x7;
    size = 0x10 + size - size%0x10;
    *(unsigned int*)(r-0x8) = size;//save size info
    top += size;
    return r;
}
