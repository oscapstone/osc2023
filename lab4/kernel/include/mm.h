#include "list.h"
#include "types.h"

#define PAGE_SHIFT	12
#define PAGE_SIZE	(1 << PAGE_SHIFT) // 4KB per page frame

#define PAGE_FRAME_NUM	(HIGH_MEMORY - LOW_MEMORY) / PAGE_SIZE
#define MAX_ORDER	6
#define MAX_ORDER_SIZE	(1 << MAX_ORDER)

#define LOW_MEMORY	0x00000000
#define HIGH_MEMORY 0x3C000000

#define FIND_BUDDY_PFN(pfn, order) ((pfn) ^ (1 << order))
#define FIND_LBUDDY_PFN(pfn, order) ((pfn) & (~(1 << order)))

#define MAX_OBJ_ALLOCTOR_NUM	16
#define MIN_ALLOCATED_OBJ_SIZE 	8
#define MAX_ALLOCATED_OBJ_SIZE	2048

#define MIN_KMALLOC_ORDER       3
#define MAX_KMALLOC_ODER        11

#define MAX_MEM_RESERVED 16

#define PFN_MASK		0x0000FFFFFFFFF000
#define PHY_ADDR_TO_PFN(addr)	((((unsigned long)(addr) - LOW_MEMORY) & PFN_MASK) >> PAGE_SHIFT)


enum booking_status {
	Free,
	Taken
};

typedef struct free_area_struct {
	int nr_free;
	struct list_head freelist;
} free_area_t;

typedef struct page {
	struct list_head list;
	int order;
	int pfn;
	int used;
	uint64_t phy_addr;

	int obj_used;
	struct object_allocator *obj_alloc;
	struct list_head *free;
} page_t;

void page_init();
void free_area_init();
void dump_buddy();

struct page* buddy_block_alloc(int order);

void buddy_block_free(struct page* block);
void push_block_to_free_area(page_t *, free_area_t *, int order);
void pop_block_from_free_area(page_t *, free_area_t *);

/**
 *  Object Allocator - Allocate memory space to object.
 *  Object allocator is based on Buudy system that are used for
 *  small object
 *
 *  @curr_page: Point to page that used to allocate memory currently
 *  @objsize: Memory size allocated by the allocator once
 *  @obj_per_page: The maximum number of obj in one page depends on objsize
 *  @obj_used: The number of objects are using in all page
 *  @page_used: The number of page are used in this allocator
 *
 */
typedef struct object_allocator {
    	struct list_head full;
    	struct list_head partial;
    	struct list_head empty;
    	struct page *curr_page;

    	int objsize;
    	int obj_per_page;
    	int obj_used;
    	int page_used;
} obj_allocator_t;

typedef struct reserved {
	// 1 for True, 0 for False
	int is_reserved;
	int start;
	int offset;
} reserved_t;

/**
 *  Initalize all members for Object allocator
 */
void __init_obj_alloc(obj_allocator_t *, int);
void __init_obj_page(page_t *);

/**
 *  Register a new obj allocator
 */
int register_obj_allocator(int);

void *obj_allocate(int token);
void obj_free(void *obj_addr);

void dump_obj_alloc(obj_allocator_t *);

/**
 *  Dynamic  Memory Allocator
 */
void __init_kmalloc();
void *kmalloc(int size);
void kfree(void *addr) ;

void mm_init();

/**
 * Reserved Memory Allocator
 * 
 */
void mem_reserved_init();
void put_memory_reserve(unsigned start, unsigned end);
struct page* reserve_memory_block(reserved_t * reserved);
void apply_memory_reserve();
void dump_mem_reserved();