#ifndef _MEM_H
#define _MEM_H

#include <type.h>

extern char _early_mem_base;
extern char _early_mem_end;

#define SMEM (&_early_mem_base)
#define EMEM (&_early_mem_end)

void *simple_malloc(uint32 size);

#endif