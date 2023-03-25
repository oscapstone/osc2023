#include "malloc.h"
#include "type.h"




extern char __heap_top;
static char* ptr = &__heap_top;

/* Simple malloc */
void* malloc(uint32_t size) {

    // 0x10 for heap_block header
    char* r = ptr + 0x10;
    
    // size paddling to multiple of 0x10
    size = size - size % 0x10;
    *(unsigned int*)(r - 0x8) = size;
    ptr += size;
    return r;

}