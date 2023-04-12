#ifndef MEMORY_H
#define MEMORY_H

#include "list.h"
#include "stdint.h"

#define DEMO 1

#define MAX_ORDER 16

/* status */
#define FREE 0
#define ALLOCATED 1

/* order */
#define NOT_BEGINNING 0

#define PAGE_SIZE 0x1000
#define MEMORY_START 0x00
#define MEMORY_END 0x3C000000


typedef struct frame {
    uint32_t order;
    uint32_t status;
} frame_entry;

typedef struct frame_entry_list_head {
    list_head_t listhead;
} frame_entry_list_head;

typedef struct chunk {
    uint32_t size;
    uint32_t status;
} chunk_entry;

typedef struct chunk_entry_list_head {
    list_head_t listhead;
} chunk_entry_list_head;

/* initialization */
void init_frames();
void init_chunks();
void init_memory();
void memory_reserve(void * start, void * end);
void init_merge_frames();
void init_chunk_listhead();

/* allocation */
void * allocate_frame(uint32_t page_num);
void * allocate_chunk(uint32_t size);

/* free */
void free_frame(void * address);
void free_chunk(void * address);

/* malloc */
void * malloc(uint32_t size);

/* utility */
uint32_t log2n(uint32_t x);
uint32_t address2idx(void *address);
void * idx2address(uint32_t idx);
int find_fit_chunk_slot(uint32_t size);

/* test */
void page_frame_allocator_test();
void chunk_slot_allocator_test();

#endif
