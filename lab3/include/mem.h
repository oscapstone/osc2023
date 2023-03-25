#ifndef __MEM_H__
#define __MEM_H__

#define STACKSIZE   0x2000
#define HEAPSIZE    0x100000

void* simple_alloc(unsigned int size);

#endif