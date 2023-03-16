#include "mem.h"
#include "muart.h"

extern char __heap_start;

static char *heap_ptr = &__heap_start;
static unsigned int space = HEAPSIZE; 

void* simple_alloc(unsigned int size) {
    char *ptr = heap_ptr;
    
    if (size > space) {
        mini_uart_puts("not enough memory to allocate\r\n"); 
    } else {
        heap_ptr += size; space -= size;
    }

    return ptr;
}