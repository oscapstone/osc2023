#ifndef __PAGE_H
#define __PAGE_H

#include "type.h"
#include "ds/list.h"

#define PAGE_BEGIN (0x10000000)
#define PAGE_END   (0x20000000)

#define PAGE_SIZE_ORDER (12)
#define PAGE_SIZE  (1 << PAGE_SIZE_ORDER)
#define PAGE_MAX_ORDER (10)

#define PAGE_CONTAINED (255)
#define PAGE_ALLOCATED (254)
#define PAGE_ALLOCATED_BY_DYNAMIC (253)

struct frame_entry {
    uint32_t idx;
    uint8_t order;
    int8_t flag;
    uint32_t dyn_count;
    uint64_t dyn_ord;
    struct mem_region *mem_region;
    struct ds_list_head head;
    struct ds_list_head chunk_head;
};

struct mem_region {
    uint64_t begin_addr;
    uint64_t size;
    uint8_t max_ord;
    struct frame_entry *entries;
    struct ds_list_head head;
};

void frame_init();
void *page_alloc(uint64_t size);
void page_free(void *ptr);
void memory_reserve(uint64_t start, uint64_t end);
struct frame_entry *get_entry_from_addr(void *ptr);

#endif