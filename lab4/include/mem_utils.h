#ifndef MEM_UTILS_H
#define MEM_UTILS_H

void* simple_malloc(unsigned int size);
void show_heap_size(void);
void* memcpy(void *dest, const void *src, int len);

#endif /* MEM_UTILS_H */