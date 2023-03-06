#include "heap.h"

extern char _heap_top;
static char* htop_ptr = &_heap_top;

// like C malloc
void* malloc(unsigned int size) {
    char* r = htop_ptr+0x10;
    if(size<0x18)size=0x18;
    size = size + 0x7;
    size = 0x10 + size - size%0x10;
    *(unsigned int*)(r+0x8) = size;
    htop_ptr += size;
    return r;
}
