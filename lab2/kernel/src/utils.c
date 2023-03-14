#include "utils.h"

void *simpleMalloc(void **current, int size)
{
    void *ret = *current;
    *current = *(char **)current + size;
    return ret;
}