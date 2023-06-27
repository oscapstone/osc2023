#ifndef __ALLOCATOR__
#define __ALLOCATOR__
#include <stdint.h>
#include <stddef.h>
void *simple_malloc(size_t size);
void malloc_init();
void *buddy_system_malloc(size_t size);
void buddy_system_free(void *tmp);
void memory_reserve(uint64_t begin, uint64_t end);
void *kmalloc(size_t size);
void kfree(void * tmp);
#endif