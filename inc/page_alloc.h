#ifndef _PAGE_ALLOC_H
#define _PAGE_ALLOC_H

#include <type.h>

extern uint64 buddy_base;
extern uint64 buddy_end;
extern uint32 frame_ents_size;

static inline int addr2idx(void *hdr)
{
    return ((uint64)hdr - buddy_base) / PAGE_SIZE;
}

static inline void *idx2addr(int idx)
{
    return (void *)(buddy_base + idx * PAGE_SIZE);
}

static inline int fls(unsigned int x)
{
    /* __builtin_clz(x) Returns the number of leading 0-bits in x, starting at the most significant bit position. 
    If x is 0, the result is undefined. */
    return x ? sizeof(x) * 8 - __builtin_clz(x) : 0;
}

void *alloc_pages(int num);
void *alloc_page();
void page_allocator_early_init(void *start, void *end);
void page_allocator_init();
void mem_reserve(void *start, void *end);
void free_page(void* page);

#ifdef DEBUG
void page_allocator_test(void);
#endif

#endif