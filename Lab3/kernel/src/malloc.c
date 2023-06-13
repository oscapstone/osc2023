#include "malloc.h"
#include "type.h"




extern char __heap_top;
static char* ptr = &__heap_top;

/* Simple malloc */
void* kmalloc(unsigned int size) {
    // -> htop_ptr
    // htop_ptr + 0x02:  heap_block size
    // htop_ptr + 0x10 ~ htop_ptr + 0x10 * k:
    //            { heap_block }
    // -> htop_ptr

    // 0x10 for heap_block header
    char* r = ptr + 0x10;
    // size paddling to multiple of 0x10
    size = 0x10 + size - size % 0x10;
    *(unsigned int*)(r - 0x8) = size;
    ptr += size;
    return r;
}


void free(void* ptr) {
    // TBD
}
