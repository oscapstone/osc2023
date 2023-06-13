#include "mm.h"
#include "math.h"
#include "memory.h"
#include "mini_uart.h"

static unsigned int n_frames = 0;
static unsigned int max_size = 0;
static struct frame* frame_list[MAX_ORDER] = {NULL};
static struct frame frame_array[(MEM_REGION_END-MEM_REGION_BEGIN)/PAGE_SIZE];
static struct dynamic_pool pools[MAX_POOLS] = { {ALLOCABLE, 0, 0, 0, 0, {NULL}, NULL} };
static unsigned int reserved_num = 0;
static void* reserved_se[MAX_RESERVABLE][2] = {{0x0, 0x0}}; // expects to be sorted and addresses [,)
extern char __kernel_end;
static char *__kernel_end_ptr = &__kernel_end;

void *malloc(unsigned int size) {

    if (size > max_size) {
        printf("[error] Request exceeded allocable continuous size %d.\n", (int)max_size);
        return NULL;
    }

    int req_order = 0;
    for(unsigned int i=PAGE_SIZE; i<size; i*=2, req_order++);

    int t;
    for (t=req_order; t<MAX_ORDER; t++) {
        if (frame_list[t] != NULL) break;
    }
    
    if (t >= MAX_ORDER) {
        printf("[error] No memory allocable.\n");
        return NULL;
    }
    
    while (t != req_order) {
        struct frame* l_tmp = frame_list[t];
        frame_list[t] = l_tmp->next;
        frame_list[t]->prev = NULL;
        //printf("[info] Split at order %d, new head is 0x%x.\n", t+1, frame_list[t]);

        unsigned int off = pow(2, l_tmp->val-1);
        struct frame* r_tmp = &frame_array[l_tmp->index+off];

        l_tmp->val -= 1;
        l_tmp->state = ALLOCABLE;
        l_tmp->prev = NULL;
        l_tmp->next = r_tmp;

        r_tmp->val = l_tmp->val;
        r_tmp->state = ALLOCABLE;
        r_tmp->prev = l_tmp;
        r_tmp->next = NULL;

        t--;
        if (frame_list[t] != NULL)
            frame_list[t]->prev = r_tmp;
        r_tmp->next = frame_list[t];
        frame_list[t] = l_tmp;
    }

    struct frame* ret = frame_list[req_order];
    frame_list[req_order] = ret->next;
    frame_list[req_order]->prev = NULL;
    
    ret->val = ret->val;
    ret->state = ALLOCATED;
    ret->prev = NULL;
    ret->next = NULL;

    //printf("[info] allocated address: 0x%x\n", MEM_REGION_BEGIN+PAGE_SIZE*ret->index);

    return (void*)MEM_REGION_BEGIN+PAGE_SIZE*ret->index;

}

void free(void *address) {

    unsigned int idx = ((unsigned long long)address-MEM_REGION_BEGIN) / PAGE_SIZE;
    struct frame* target = &frame_array[idx];

    if (target->state == ALLOCABLE || target->state == C_NALLOCABLE) {
        printf("[error] invalid free of already freed memory.\n");
        return;
    }
    //printf("=========================================================\n");
    //printf("[info] Now freeing address 0x%x with frame index %d.\n", address, (int)idx);

    for (int i=target->val; i<MAX_ORDER; i++) {
        
        unsigned int buddy = idx ^ (unsigned int)pow(2, i);
        struct frame* fr_buddy = &frame_array[buddy];
        //printf("[info] Index %d at order %d, buddy %d at order %d state %d.\n", 
        //        (int)idx, (int)i+1, (int)buddy, fr_buddy->val+1, fr_buddy->state);

        if (i < MAX_ORDER-1 && fr_buddy->state == ALLOCABLE && i== fr_buddy->val) {
            
            //printf("[info] Merging from order %d. Frame indices %d, %d.\n", i+1, (int)buddy, (int)idx);
            
            if (fr_buddy->prev != NULL) {
                fr_buddy->prev->next = fr_buddy->next;
            } else {
                frame_list[fr_buddy->val] = fr_buddy->next;
            }

            if (fr_buddy->next != NULL) {
                fr_buddy->next->prev = fr_buddy->prev;
            }

            fr_buddy->prev = NULL;
            fr_buddy->next = NULL;
            fr_buddy->val = C_NALLOCABLE;
            fr_buddy->state = C_NALLOCABLE;
            target->val = C_NALLOCABLE;
            target->state = C_NALLOCABLE;

            if (fr_buddy->index < target->index) {
                idx = fr_buddy->index;
                target = fr_buddy;
            }
            
            //printf("[info] Frame index of next merge target is %d.\n", (int)idx);

        } else {

            target->val = i;
            target->state = ALLOCABLE;
            target->prev = NULL;
            target->next = frame_list[i];
            if (frame_list[i] != NULL)
                frame_list[i]->prev = target;
            frame_list[i] = target;
            //printf("[info] Frame index %d pushed to frame list of order %d.\n", 
            //    (int)target->index, (int)i+1);
            break;

        }
        
    } 

    //printf("[info] Free finished.\n");
    /*for (int i=0; i < MAX_ORDER; i++) {
        if (frame_list[i] != NULL)
            printf("[info] Head of order %d has frame array index %d.\n",i+1,frame_list[i]->index);
        else
            printf("[info] Head of order %d has frame array index null.\n",i+1);
    }*/

}

void init_mm() {

    n_frames = (MEM_REGION_END-MEM_REGION_BEGIN) / PAGE_SIZE;
    unsigned int mul = (unsigned int)pow(2, MAX_ORDER-1);
    printf("[info] Frame array start address 0x%x.\n", frame_array);
    for (unsigned int i=0; i<n_frames; i++) {
        frame_array[i].index = i;
        if (i%mul == 0) {
            frame_array[i].val = MAX_ORDER-1;
            frame_array[i].state = ALLOCABLE;
            frame_array[i].prev = &frame_array[i-mul];
            frame_array[i].next = &frame_array[i+mul];
        } else {
            frame_array[i].val = C_NALLOCABLE;
            frame_array[i].state = C_NALLOCABLE;
            frame_array[i].prev = NULL;
            frame_array[i].next = NULL;
        }
    }
    frame_array[0].prev = NULL;
    frame_array[n_frames-mul].next = NULL;

    for (int i=0; i<MAX_ORDER; i++) {
        frame_list[i] = NULL;
    }

    frame_list[5] = &frame_array[0];
    
    max_size = PAGE_SIZE * pow(2, MAX_ORDER-1);

}

void init_pool(struct dynamic_pool* pool, unsigned int size) {
    pool->chunk_size = size;
    pool->chunks_per_page = PAGE_SIZE / size;
    pool->chunks_allocated = 0;
    pool->page_new_chunk_off = 0;
    pool->pages_used = 0;
    pool->free_head = NULL;
}

int register_chunk(unsigned int size) {
    
    unsigned int nsize = 0;
    if (size <= 8) nsize = 8;
    else {
        int rem = size % 4;
        if (rem != 0) nsize = (size/4 + 1)*4;
        else nsize = size;
    }

    if (nsize >= PAGE_SIZE) {
        printf("[error] Normalized chunk size request leq page size.\n");
        return -1;
    }

    for (int i=0; i<MAX_POOLS; i++) {
        if (pools[i].chunk_size == nsize) return i;
        else if (pools[i].chunk_size == ALLOCABLE) {
            init_pool(&pools[i], nsize);
            return i;
        }
    }

    return -1;

}

void *chunk_alloc(unsigned int size) {

    int pool_idx = register_chunk(size);
    //printf("[info] pool index is %d.\n", pool_idx);
    if (pool_idx == -1) return NULL;
    
    struct dynamic_pool* pool = &pools[pool_idx];

    if (pool->free_head != NULL) {
        void *ret = (void*) pool->free_head;
        pool->free_head = pool->free_head->next;
        //printf("[info] allocate address 0x%x from pool free list.\n", ret);
        return ret;
    }

    if (pool->chunks_allocated >= MAX_POOL_PAGES*pool->chunks_per_page) {
        //printf("[error] Pool maximum reached.\n");
        return NULL;
    }
        

    if (pool->chunks_allocated >= pool->pages_used*pool->chunks_per_page) {
        pool->page_base_addrs[pool->pages_used] = malloc(PAGE_SIZE);
        //printf("[info] allocate new page for pool with base address 0x%x.\n", 
        //        pool->page_base_addrs[pool->pages_used]);
        pool->pages_used++;
        pool->page_new_chunk_off = 0;
    }

    void *ret = pool->page_base_addrs[pool->pages_used - 1] + 
                pool->chunk_size*pool->page_new_chunk_off;
    pool->page_new_chunk_off++;
    pool->chunks_allocated++;

    //printf("[info] allocate new address 0x%x from pool.\n", ret);

    return ret;

}

void chunk_free(void *address) {

    int target = -1;

    void *prefix_addr = (void *)((unsigned long long)address & ~0xFFF);

    for (unsigned int i=0; i<MAX_POOLS; i++) {
        for (unsigned int j=0; j<pools[i].pages_used; j++) {
            void *base = pools[i].page_base_addrs[j];
            if (base == prefix_addr)
                target = i;
        }
    }
    //printf("[info] free chunk from pool %d.\n", target);
    struct dynamic_pool *pool = &pools[target];
    struct node* old_head = pool->free_head;
    pool->free_head = (struct node*) address;
    pool->free_head->next = old_head;
    pool->chunks_allocated--;

}

void memory_reserve(void* start, void* end) {
    if (reserved_num >= MAX_RESERVABLE) {
        printf("[error] Max reservable locations already reached.\n");
        return;
    }
    reserved_se[reserved_num][0] = start;
    reserved_se[reserved_num][1] = end;
    reserved_num++;
}

void init_mm_reserve() {

    max_size = PAGE_SIZE * pow(2, MAX_ORDER-1);
    n_frames = (MEM_REGION_END-MEM_REGION_BEGIN) / PAGE_SIZE;

    memory_reserve((void*)0x0, __kernel_end_ptr); // spin tables, kernel image
    memory_reserve((void*)0x20000000, (void*)0x20010000); // hard code reserve initramfs
    
    for (unsigned int i=0; i<n_frames; i++) {
        frame_array[i].index = i;
        frame_array[i].val = 0;
        frame_array[i].state = ALLOCABLE;
        frame_array[i].prev = NULL;
        frame_array[i].next = NULL;
    }

    int j = 0;
    for (unsigned int i=0; i<reserved_num; i++) {
        for (; j<n_frames; j++) {
            void *addr = (void*)MEM_REGION_BEGIN + PAGE_SIZE*j;
            if (addr >= reserved_se[i][0] && addr < reserved_se[i][1]) {
                frame_array[j].state = RESERVED;
            }
            if (addr >= reserved_se[i][1]) break;
        }
    }

    for (int i=0; i<MAX_ORDER; i++) {
        frame_list[i] = NULL;
    }

    for (unsigned int n=0; n<n_frames; n++) {

        struct frame* target = &frame_array[n];
        unsigned int idx = n;

        if (target->state == RESERVED) continue;
        if (target->state == C_NALLOCABLE) continue;

        for (int i=target->val; i<MAX_ORDER; i++) {
        
            unsigned int buddy = idx ^ (unsigned int)pow(2, i);
            struct frame* fr_buddy = &frame_array[buddy];

            if (i < MAX_ORDER-1 && fr_buddy->state == ALLOCABLE && i== fr_buddy->val) {
                
                if (fr_buddy->prev != NULL) {
                    fr_buddy->prev->next = fr_buddy->next;
                } else {
                    frame_list[fr_buddy->val] = fr_buddy->next;
                }

                if (fr_buddy->next != NULL) {
                    fr_buddy->next->prev = fr_buddy->prev;
                }

                fr_buddy->prev = NULL;
                fr_buddy->next = NULL;
                fr_buddy->val = C_NALLOCABLE;
                fr_buddy->state = C_NALLOCABLE;
                target->val = C_NALLOCABLE;
                target->state = C_NALLOCABLE;

                if (fr_buddy->index < target->index) {
                    idx = fr_buddy->index;
                    target = fr_buddy;
                }

            } else {

                target->val = i;
                target->state = ALLOCABLE;
                target->prev = NULL;
                target->next = frame_list[i];
                if (frame_list[i] != NULL)
                    frame_list[i]->prev = target;
                frame_list[i] = target;
                break;

            }
            
        } 

    }

}