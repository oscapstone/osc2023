#include "salloc.h"
#include "uart.h"
#include "utils.h"
//https://sourceware.org/binutils/docs/ld/Source-Code-Reference.html
//defined in linker script
extern char __heap_start, __heap_end;

void *simple_malloc(unsigned long long size) {
    static unsigned long long allocated = 0;
    if (allocated + size > (& __heap_end - & __heap_start)) {
        uart_write_string("heap is full!\n");
        return NULL;
    }
    char *ret = & __heap_start + allocated;
    allocated += size;
    return ret;
}