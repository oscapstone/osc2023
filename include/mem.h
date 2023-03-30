
#ifndef __MEM_H
#define __MEM_H
#include "type.h"

#define NULL (0)

void *simple_malloc(uint32_t size);
void free(void *ptr);

#endif