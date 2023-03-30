#include "init/salloc.h"

/*
 * Simple memory allocator. Use this during kernel initialization only.
 * Allocate continuous memory space from 0x40000 to 0x59999.
 */

#define SHEAP_LIMIT ((char *)0x59999U)

void * simple_malloc(unsigned int size) {
    static char * avai = (char *)0x40000U;
    if (avai + size > SHEAP_LIMIT) {
        return SALLOC_EALLOC;
    }
    char * alloc = avai;
    avai += size;
    return alloc;
}
