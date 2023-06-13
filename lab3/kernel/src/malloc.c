#include "malloc.h"

extern char _heap_top;
static char* htop_ptr = &_heap_top;

void* malloc(unsigned int size) {

    // keep 0x10 for heap_block header
    char* r = htop_ptr + 0x10;
    // size paddling to multiple of 0x10
    size = size - size % 0x10 + 0x10; //calculate number of heap blocks (無條件進位)
    
    *(unsigned int*)(r - 0x8) = size; //put size in header.
    htop_ptr += size;
    return r; //base address to use. 
}

void free(void* ptr) {
    
}
