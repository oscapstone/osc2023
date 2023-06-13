#include "memory.h"
#include "mini_uart.h"

extern char __heap_start;

static char *__heap_ptr = &__heap_start;
static unsigned int avail = MAX_HEAP_SIZE;

void* simple_malloc(unsigned int size) {

    char *ptr = __heap_ptr;

    if (size > avail) {
        uart_send_string("not enough memory\n");
    } else {
        __heap_ptr += size;
        avail -= size;
    }

    return ptr;

}