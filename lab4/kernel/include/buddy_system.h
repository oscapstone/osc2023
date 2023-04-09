#ifndef BUDDY_SYSTEM_H
#define BUDDY_SYSTEM_H
#include "list.h"

#define BUDDY_MEMORY_BASE       0x0     // 0x10000000 - 0x20000000 (SPEC) -> Advanced #3 for all memory region
#define BUDDY_MEMORY_PAGE_COUNT 0x3C000 // let BUDDY_MEMORY use 0x0 ~ 0x3C000000 (SPEC)

void* kmalloc(unsigned int size);
void buddy_system_init();
unsigned long int buddy_system_alloc(int size);
void buddy_system_free(int index);
void reserve_memory(unsigned long long start, unsigned long long end);

#endif /* BUDDY_SYSTEM_H */