#ifndef	_MM_H
#define	_MM_H

#define MEM_REGION_BEGIN    0x0
#define MEM_REGION_END      0x3C000000
#define PAGE_SIZE           4096
#define MAX_ORDER           8 // largest: PAGE_SIZE*2^(MAX_ORDER)

#define ALLOCABLE           0
#define ALLOCATED           -1
#define C_NALLOCABLE        -2
#define RESERVED            -3

#define NULL                0

#define MAX_POOL_PAGES      8
#define MAX_POOLS           8
#define MIN_CHUNK_SIZE      8

#define MAX_RESERVABLE      8

struct frame {
    unsigned int index;
    int val;
    int state;
    struct frame *prev, *next;    
};

struct node {
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

void memzero(unsigned long, unsigned long);

#endif  /*_MM_H */