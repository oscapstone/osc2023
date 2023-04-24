#ifndef	_MM_H
#define	_MM_H

#define MEM_REGION_BEGIN    0x0             // 0x10000000 - 0x20000000 
#define MEM_REGION_END      0x3C000000      // let BUDDY_MEMORY use 0x0 ~ 0x3C000000 (SPEC)
#define PAGE_SIZE           4096            //4KB, 0x1000
#define MAX_ORDER           6               // largest: PAGE_SIZE*2^(MAX_ORDER)
//#define MAX_PAGES         0x10000     // 65536 (Entries), PAGESIZE * MAX_PAGES = 0x10000000 (SPEC)

#define ALLOCABLE           0
#define ALLOCATED           -1
#define C_NALLOCABLE        -2
#define RESERVED            -3

#define NULL                0

#define MAX_POOL_PAGES      8
#define MAX_POOLS           8
#define MIN_CHUNK_SIZE      8

#define MAX_RESERVABLE      8

struct frame{
    unsigned int index;
    int val;        
    int state;      
    struct frame *prev, *next;
};


struct node{
    struct node *next;
};

struct dynamic_pool {
    unsigned int chunk_size;
    unsigned int chunks_per_page;
    unsigned int chunks_allocated;
    unsigned int page_new_chunk_off;
    unsigned int pages_used;
    void *page_base_addrs[MAX_POOL_PAGES];
    struct node *free_head;
};

void *malloc(unsigned int);
void free(void *);
void init_mm();
void init_pool(struct dynamic_pool*, unsigned int);
int register_chunk(unsigned int);
void *chunk_alloc(unsigned int);
void chunk_free(void *);
void memory_reserve(void*, void*);
void init_mm_reserve();



#endif  /*_MM_H */