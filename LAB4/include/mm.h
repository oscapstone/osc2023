#ifndef _MM_H
#define _MM_H

#include <stdint.h>
#include <stddef.h>

#define STARTUP_MEM_START 0x07000000
#define STARTUP_MEM_END 0x077fffff

typedef struct FrameFlag
{
    unsigned char flag;
    unsigned char order;
    unsigned short ref_count;
    // 該frame 有被切成 chunk才會用到
    unsigned char chunk_order;
} FrameFlag;
// 且Chunk是singly linked list
typedef struct Chunk
{
    struct Chunk *next;
} Chunk;

// buddy system
void init_buddy();
void *alloc_pages(unsigned int pages);
void free_pages(void *victim);

// dynamic mem allocate
void *kmalloc(unsigned int size);
void kfree(void *ptr);

// test
void test_buddy();
void test_dynamic_alloc();

// others
void memory_reserve(uintptr_t start, uintptr_t end);
void mm_init();
void merge_useful_pages();

// start up allocation
void *smalloc(size_t size);

#endif