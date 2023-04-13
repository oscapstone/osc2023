#ifndef _MM_H
#define _MM_H

#include "list.h"
#include "types.h"

// refrence: https://www.kernel.org/doc/gorman/html/understand/understand009.html
#define PAGE_SHIFT_BIT 12
#define PAGE_SIZE (1 << PAGE_SHIFT_BIT) //4096
#define MAX_ORDER 6 // Levels of Order , 2^6 is 64 pages = 0x40000 per Freelist object
#define MAX_ORDER_SIZE (1 << MAX_ORDER) // Total size of buddy system

#define PAGE_FRAME_NUM (BUDDY_HI - BUDDY_LO)/(PAGE_SIZE)
#define BUDDY_LO 0x00000000
#define BUDDY_HI 0x3C000000

#define FIND_BUDDY_PFN(pfn, order) ((pfn) ^ (1<<(order)))
#define FIND_LBUDDY_PFN(pfn, order)((pfn) & (~(1<<(order))))

#define MAX_DYNAMIC_ALLOC_NUM 16
#define MIN_DYN_SIZE 8
#define MAX_DYN_SIZE 2048

#define MAX_MEM_RESERVED 16

#define PFN_MASK 0x0000FFFFFFFFF000
#define PHY_ADDR_TO_PFN(addr) (((((unsigned long)(addr)) - BUDDY_LO) & PFN_MASK) >> 12)

enum page_status {
	Free,
	Taken
};

typedef struct free_area_struct {
	struct list_head free_list;
	unsigned long map;
 } free_area_t;

typedef struct page {
    struct list_head list;

    int order;
    int pfn; // page frame number
    int used;
	uint64_t phy_addr;

	int obj_used;
	struct object_allocator * object_alloc;
	struct list_head * free;
 } page_t;

typedef struct object_allocator{
	struct list_head full;
	struct list_head partial;
	struct list_head empty;
	struct page *curr;

	int objsize;
	int obj_per_page;
	int obj_used;
	int page_used;
} object_allocator_t;

typedef struct reserved {
	// 1 for True, 0 for False
	int is_reserved;
	int start;
	int offset;
} reserved_t;

void page_init();
void free_area_init();
void push2free(page_t *page, free_area_t *free_area, int order);
void pop_free(page_t *page, free_area_t * freearea);
void buddy_free(struct page * page);
void buddy_init();

void obj_alloc_init(object_allocator_t * obj_alloc, int objsize);
void obj_page_init(page_t *page);
int obj_alloc_reg(int objsize);
void * obj_allocate(int token);
void obj_free(void * obj_addr);
void dyn_init();
void obj_free(void * obj_addr);

void dump_free_area();
void dump_dyn_area(object_allocator_t * obj_alloc);

void mem_reserved_init();
void put_memory_reserve(unsigned start, unsigned end);
struct page * reserve_memory_block(reserved_t * reserved);
void apply_memory_reserve();
void dump_mem_reserved();
#endif
