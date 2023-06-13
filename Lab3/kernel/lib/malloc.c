#include "uart.h"
#include "malloc.h"

extern char __heap_start;
char *__heap_top = (char *) &__heap_start;

void *simple_malloc(unsigned long size)
{
    uart_printf("malloc %x\n", __heap_top);
    char *r = __heap_top;
    size = (size + 0x10 - 1) / 0x10 * 0x10;
    __heap_top += size;
    return r;
}