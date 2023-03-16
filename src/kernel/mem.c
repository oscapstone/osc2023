#include "mem.h"
#include "peripherals/mini_uart.h"

extern uint32_t _heap_begin;
void* cur_mem_offset = &_heap_begin;

void *simple_malloc(uint32_t size) {
    // pad to align 8 bytes
    uint32_t offset = ((((size & 0x7) ^ 0x7) + 1) & 0x7) + size;
    cur_mem_offset += offset;
    return (cur_mem_offset - offset);
}