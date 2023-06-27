#include <allocator.h>

//#include <uart.h>

extern uint32_t _heap_begin;
extern uint32_t _heap_end;

void *mem_chunk_begin;
void *mem_chunk_end;

int check;

void *simple_malloc(size_t size){
    if(check != 1){
        check = 1;
        mem_chunk_begin = (void *) &_heap_begin;
        mem_chunk_end = (void *) &_heap_end;
    }
    size = (((size - 1) >> 4) + 1) << 4;
    if((uint64_t) mem_chunk_begin + size >= (uint64_t) mem_chunk_end){
        return NULL;
    }
    void *chunk = mem_chunk_begin;
    mem_chunk_begin = (void *)((uint64_t) mem_chunk_begin + size);
    return chunk;
}