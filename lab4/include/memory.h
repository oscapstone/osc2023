#ifndef MEMORY_H
#define MEMORY_H

#define MEMORY_START 0x0

#define PAGE_SHIFT 12
#define PAGE_SIZE (1 << PAGE_SHIFT)           // 4096
#define MAX_BUDDY_ORDER 9                     //
#define MAX_BLOCK_SIZE (1 << MAX_BUDDY_ORDER) // 512
#define MAX_PAGE_NUMBER 4096

#define MIN_OBJECT_ORDER 4                                           // 16 bytes
#define MAX_OBJECT_ORDER 11                                          // 2048 bytes
#define MIN_OBJECT_SIZE (1 << MIN_OBJECT_ORDER)                      // 16
#define MAX_OBJECT_SIZE (1 << MAX_OBJECT_ORDER)                      // 2048
#define MAX_ALLOCATOR_NUMBER MAX_OBJECT_ORDER - MIN_OBJECT_ORDER + 1 // 8

static void *NULL = 0;

#include "list.h"

struct page
{
    struct list_head list; // must be put in the front

    int order; // 0-9

    void *first_free;                   // first free address in this page
    struct object_allocator *allocator; // which allocator this page belongs to
    int object_count;                   // how many objects this page stores currently

    int page_number; // the number of this page
    int used;        // 0: not used, 1: used
    void *start_address;
};

struct object_allocator
{
    struct page *current_page;
    // struct page *preserved_empty;
    struct list_head partial;
    struct list_head full;
    int max_object_count; // how many objects pages controlled by this allocator can store
    int object_size;
};

void init_buddy();
void init_object_allocator();
void init_memory();

struct page *block_allocation(int order);
void block_free(struct page *block);
void *object_allocation(int token);
void object_free(void *object);

void *memory_allocation(int size);
void memory_free(void *address);

int find_buddy(int page_number, int order);

void memory_reserve(void *start, void *end);
#endif