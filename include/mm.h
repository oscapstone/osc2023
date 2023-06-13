#ifndef	_MM_H
#define	_MM_H
#include "mmu.h"
#include "utils.h"

#define PAGE_SHIFT	 		12
#define TABLE_SHIFT 			9
#define SECTION_SHIFT			(PAGE_SHIFT + TABLE_SHIFT)

#define PAGE_SIZE   			(1 << PAGE_SHIFT)
#define PAGE_MASK               (~(PAGE_SIZE-1))
#define SECTION_SIZE			(1 << SECTION_SHIFT)	

#define LOW_MEMORY              	(2 * SECTION_SIZE)

#ifndef __ASSEMBLER__
extern void my_bzero(void *src, unsigned long n);

#define MEM_START PA2VA(0x0)
#define MEM_END PA2VA(0x3C000000)
#define PAGE_CNT ((MEM_END - MEM_START) >> PAGE_SHIFT)
////////////////////////// startup allocator //////////////////////
extern void *simple_malloc(unsigned long long size);

////////////////////////// buddy system ////////////////////////
// #define PRINT_LOG 1
#define BUDDY_ORDERS 16
#define BLK_INVALID (~(unsigned)0)
#define BLK_INUSE(blk) ( GETBIT((blk).flags, (sizeof(unsigned) << 3)-1) )
#define BLK_ISFREE(blk) (!BLK_INUSE(blk))
#define BLK_ORDER(blk) ( (blk).flags & 0xf )
#define SET_INUSE(blk) ( SETBIT((blk).flags, (sizeof(unsigned) << 3)-1) )
#define SET_FREE(blk) ( CLRBIT((blk).flags, (sizeof(unsigned) << 3)-1) )
//                                               clear last 4 bit
#define SET_ORDER(blk, order) ( (blk).flags = ( (((blk).flags) & ~(0xf)) | (order & 0xf) ) )
typedef struct buddy_block {
    unsigned flags;//last 4 bit stores order, first bit state in_use
    unsigned next_free_index;//used to link free blocks of the same order
    unsigned prev_free_index;//used to link free blocks of the same order
} bd_blk_t;
#define index2addr(self, index) ((self)->pool + ((index) * PAGE_SIZE))
#define addr2index(self, addr) ( ((uint64_t)(addr) - (uint64_t)((self)->pool)) >> PAGE_SHIFT )
typedef struct buddy {
    //The Array in the lab spec.
    // bd_blk_t blk_meta[(1 << (BUDDY_ORDERS))];
    bd_blk_t *blk_meta;
    size_t free_head[BUDDY_ORDERS];
    size_t link_size[BUDDY_ORDERS];
    char *pool;
    char *pool_end;
    void *(*alloc_pages)(struct buddy *self, size_t no_pages);
    void (*free)(struct buddy *self, void *addr);
    void (*mem_reserve)(struct buddy *self, char *start, char *end);
} buddy_t;
extern void init_buddy(buddy_t *self);
extern buddy_t _buddy;
extern void *alloc_pages(size_t no_pages);
extern void free_page(void *addr);
#define NEEDED_PAGES(size) (ceil((size), PAGE_SIZE))
///////////////////// dynamic memory allocator //////////////////
//slab memory
typedef struct mem_chunk {
    //address of mem_chunk should be aligned to 16.
    //next is defined as (next address of the same order | order) //depreciated
    struct mem_chunk *next;
    size_t order;
    char data[0];
} mem_chunk_t;
#define CHUNK_ORDERS 128
// #define SET_CHUNK_ORDER(chunk, order) ((chunk)->next = (mem_chunk_t *)(((uint64_t)((chunk)->next) & ~(CHUNK_ORDERS-1)) | ((order) & (CHUNK_ORDERS-1))))
#define SET_CHUNK_ORDER(chunk, set_order) ((chunk)->order = (set_order))
// #define GET_CHUNK_ORDER(chunk) ((uint64_t)((chunk)->next) & (CHUNK_ORDERS-1))
#define GET_CHUNK_ORDER(chunk) ((chunk)->order)
// #define GET_CHUNK_NEXT(chunk) (mem_chunk_t *) ((uint64_t)((chunk)->next) & ~(CHUNK_ORDERS-1))
#define GET_CHUNK_NEXT(chunk) ((chunk)->next)
// #define CLR_CHUNK_NEXT(chunk) (((chunk)->next) = (mem_chunk_t *) ((uint64_t)((chunk)->next) & (CHUNK_ORDERS-1)))
#define CLR_CHUNK_NEXT(chunk) ((chunk)->next = NULL)
typedef struct mem_pool {
    // void *chunk_pools[CHUNK_ORDERS];
    //address within doesn't include order
    mem_chunk_t *free_list[CHUNK_ORDERS];
    size_t chunk_orders;
    void *(*malloc)(struct mem_pool *self, size_t size);
    void (*free)(struct mem_pool *self, void *addr);
} mem_pool_t;
extern void init_mem_pool(mem_pool_t* self);
extern mem_pool_t _mem_pool;
// #define malloc(size) _mem_pool.malloc(&_mem_pool, (size))
// #define free(addr) _mem_pool.free(&_mem_pool.free, (addr))
extern void *kmalloc(size_t size);
extern void kfree(void *addr);
extern void test_buddy();
extern void test_mem_pool();

#endif
#endif  /*_MM_H */