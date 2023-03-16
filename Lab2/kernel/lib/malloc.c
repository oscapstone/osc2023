#include "uart.h"

extern char __heap_start;
char *__heap_top = (char *) &__heap_start;
// char *__heap_top = (char *) 0x10000000;

void *simple_malloc(unsigned long size)
{
    char *r = __heap_top;
    __heap_top += size;
    return r;
}