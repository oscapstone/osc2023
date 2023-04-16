#ifndef _MEM_H
#define _MEM_H

#include <type.h>

extern char start_mem;
extern char end_mem;

#define SMEM (&start_mem)
#define EMEM (&end_mem)

void *simple_malloc(uint32 size);

#endif