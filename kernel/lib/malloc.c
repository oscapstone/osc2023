#include "malloc.h"

char *__heap_top = &__heap_start;

void *simple_malloc(unsigned long size)
{
    char *r = __heap_top;
    __heap_top += size;
    return r;
}