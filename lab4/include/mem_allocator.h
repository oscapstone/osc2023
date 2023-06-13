#ifndef MEM_ALLOCATOR_H
#define MEM_ALLOCATOR_H

#define PAGE_HEADER_SIZE        32
#define MIN_CHUNK_ORDER         5
#define NUM_CHUNK_SIZE          6
#define MAX_CHUNK_SIZE          1024

#define NUM_CHUNK_32    3
#define NUM_CHUNK_64    4
#define NUM_CHUNK_128   1
#define NUM_CHUNK_256   4
#define NUM_CHUNK_512   3
#define NUM_CHUNK_1024  1

void init_allocator(void);
void* malloc(unsigned int size);
void free(void* ptr);

void demo_dynamic_allocation(void);

#endif /* MEM_ALLOCATOR_H */