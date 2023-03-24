#include "malloc.h"
#include "uart.h"

extern char __heap_start;
extern char __heap_end;
static char *heap_top = &__heap_start;

void *malloc(unsigned int size)
{
    char *r = heap_top;

    if (heap_top + size > &__heap_end)
    {
        uart_puts("heap overflow!\n");
        return;
    }

    heap_top += size;

    return r;
}