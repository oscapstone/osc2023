#include "malloc.h"


/*
                        Chunk

        | ----------------------------------------- |
        |   prev_size/data    |    chunk_size       |
        |     (0x8 bytes)     |    (0x8 bytes)      |
        | ------------------------------------------|
        |                ...data...                 |
        |                                           |
        | ------------------------------------------|

    ps. chunk size : prev_size + chunk_size + data, ( 0x10 alignment )
        malloc size : data

*/

extern char __heap_top;
static char* ptr = &__heap_top;

/* Simple malloc */
void* malloc(uint32_t size) {

    char* ret = ptr + sizeof(malloc_chunk);

    // The smallest chunk is 0x20 -> at least 0x18 can be used
    if (size < 0x18) size = 0x18;

    /* Size paddling to multiple of 0x10 */
    size = ((size + 0x0F) & ~0x0F);

    /* Sets Size value to Chunk_Size */
    ((malloc_chunk*)ret)->chunk_size = size;

    ptr += size;

    return ret;
}