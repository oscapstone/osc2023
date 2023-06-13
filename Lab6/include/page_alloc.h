#ifndef _PAGE_ALLOC_H
#define _PAGE_ALLOC_H

// #define DEBUG

#include "virtual_mem.h"

#define MAX_ORDER 11 // order 0 ~ 11
#define ALLOCATED -1
#define FREE_BUDDY 99
#define MIN_PAGE_SIZE 0x1000
// #define FREE_MEM_START 0x10000000
// #define FREE_MEM_END 0x20000000
#define FREE_MEM_START KERNEL_PA_TO_VA(0x00)
#define FREE_MEM_END KERNEL_PA_TO_VA(0x3C000000)
#define TOTAL_NUM_PAGE (FREE_MEM_END - FREE_MEM_START) / MIN_PAGE_SIZE

typedef struct _page_frame_node page_frame_node;
struct _page_frame_node
{
    int index; // const
    int val;
    void *addr;          // const
    int contiguous_head; // if allocated, this value keep track who (page) is the start of the contiguous memory in index
    int allocated_order;
    int chunk_order; // -1 if no chunk, otherwise 1, 2, 4, 6, 8, 16, 32
    page_frame_node *next;
    page_frame_node *previous;
};

void init_page_frame();
void *my_malloc(int req_size);
int get_page_from_free_list(int req_size, int who);
void add_to_free_list(page_frame_node *head_node, int index);
void remove_from_free_list(page_frame_node *free_list_node);
void put_back_to_free_list(int num_of_redundant_page, int index);
int free_page_frame(int index);
int merge_buddy(int *index, int buddy, int order);
void debug();

#endif /*_PAGE_ALLOC_H */
