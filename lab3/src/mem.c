#include "mem.h"
#include "mini_uart.h"


extern char heap_start;
extern char heap_end;

static char *heap_start_ptr = &heap_start;
static char *heap_end_ptr = &heap_end;

void *simple_malloc(unsigned int size) {
    void *mem;

    mem = heap_start_ptr;

    if(heap_start_ptr + size >= heap_end_ptr) {
        uart_puts("Segmentation fault!\r\n");
        return 0;
    }

    // uart_puts("Space allocated: ");
    // uart_ptr(mem);
    // uart_puts(" ~ ");
    // uart_ptr(mem + size);
    // uart_puts("\r\n");

    heap_start_ptr += size;
    // uart_puts("Success malloc!\r\n");

    return mem;
}