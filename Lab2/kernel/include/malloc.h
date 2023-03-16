#include "type.h"

#ifndef __MEM__H__
#define __MEM_H__



/*

The memory unit allocated by malloc is chunk

                    Chunk

| ----------------------------------------- |
| prev_size/prev_data |       size          |
|       (8 bytes)     |    (8 bytes)        |
| ------------------------------------------|
|                ...data...                 |
|                                           |
| ------------------------------------------|
*/


typedef struct malloc_chunk {
    uint32_t prev_size;
    uint32_t chunk_size;

    /* forward pointer */
    //struct chunk *fd;  
    /* backward pointer */
    //struct chunk *bk;   
} malloc_chunk;

void* malloc(uint32_t);

#endif