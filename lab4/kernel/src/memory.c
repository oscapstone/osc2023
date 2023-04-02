#include "memory.h"
#include "u_list.h"
#include "uart1.h"
#include "exception.h"

// ------ Lab2 ------
extern char _heap_top;
static char* htop_ptr = &_heap_top;

void* s_allocator(unsigned int size) {
    // -> htop_ptr
    // htop_ptr + 0x02:  heap_block size
    // htop_ptr + 0x10 ~ htop_ptr + 0x10 * k:
    //            { heap_block }
    // -> htop_ptr

    // 0x10 for heap_block header
    char* r = htop_ptr + 0x10;
    // size paddling to multiple of 0x10
    size = 0x10 + size - size % 0x10;
    *(unsigned int*)(r - 0x8) = size;
    htop_ptr += size;
    return r;
}

void s_free(void* ptr) {
    // TBD
}

// ------ Lab4 ------
static frame_t            frame_array[MAX_PAGES] = {0};
static list_head_t        frame_freelist[FRAME_MAX_IDX];
static list_head_t        cache_list[CACHE_MAX_IDX];

void init_allocator()
{
    // init frame_array
    for (int i = 0; i < MAX_PAGES; i++)
    {
        if (i % (1 << FRAME_IDX_FINAL) == 0)
        {
            frame_array[i].val = FRAME_IDX_FINAL;
            frame_array[i].used = FRAME_FREE;
        }
    }

    //init frame freelist
    for (int i = FRAME_IDX_0; i <= FRAME_IDX_FINAL; i++)
    {
        INIT_LIST_HEAD(&frame_freelist[i]);
    }

    //init cache list
    for (int i = CACHE_IDX_0; i<= CACHE_IDX_FINAL; i++)
    {
        INIT_LIST_HEAD(&cache_list[i]);
    }

    for (int i = 0; i < MAX_PAGES; i++)
    {
        //init listhead for each frame
        INIT_LIST_HEAD(&frame_array[i].listhead);
        frame_array[i].idx = i;
        frame_array[i].cache_order = CACHE_NONE;

        //add init frame into freelist
        if (i % (1 << FRAME_IDX_FINAL) == 0)
        {
            list_add(&frame_array[i].listhead, &frame_freelist[FRAME_IDX_FINAL]);
        }
    }
}

//smallest 4K
void* page_malloc(unsigned int size)
{
    uart_sendline("    [+] Allocate page - size : %d(0x%x)\r\n", size, size);
    uart_sendline("        Before\r\n");
    dump_page_info();

    // get real val size
    //int allocsize;
    int val;
    for (int i = FRAME_IDX_0; i <= FRAME_IDX_FINAL; i++)
    {

        if (size <= (PAGESIZE << i))
        {
            val = i;
            uart_sendline("        block size = 0x%x\n", PAGESIZE << i);
            break;
        }

        if ( i == FRAME_IDX_FINAL)
        {
            uart_puts("[!] request size exceeded for page_malloc!!!!\r\n");
            return (void*)0;
        }

    }

    // find the smallest larger frame in freelist
    int target_val;
    for (target_val = val; target_val <= FRAME_IDX_FINAL; target_val++)
    {
        // freelist does not have 2**i order frame, going for next order
        if (!list_empty(&frame_freelist[target_val]))
            break;
    }

    if (target_val > FRAME_IDX_FINAL)
    {
        uart_puts("[!] No available frame in freelist, page_malloc ERROR!!!!\r\n");
        return (void*)0;
    }
    // get the frame from freelist
    frame_t *target_frame_ptr = (frame_t*)frame_freelist[target_val].next;
    list_del_entry((struct list_head *)target_frame_ptr);

    // Release redundant memory block to separate into pieces
    for (int j = target_val; j > val; j--) // ex: 10000 -> 01111
    {
        release_redundant(target_frame_ptr);
    }
    target_frame_ptr->used = FRAME_ALLOCATED;
    uart_sendline("        physical address : 0x%x\n", BUDDY_MEMORY_BASE + (PAGESIZE*(target_frame_ptr->idx)));
    uart_sendline("        After\r\n");
    dump_page_info();

    return (void *) BUDDY_MEMORY_BASE + (PAGESIZE * (target_frame_ptr->idx));
}

void page_free(void* ptr)
{
    frame_t *target_frame_ptr = &frame_array[((unsigned long long)ptr - BUDDY_MEMORY_BASE) >> 12]; // MAX_PAGES * 64bit -> 0x1000 * 0x10000000
    uart_sendline("    [+] Free page: 0x%x, val = %d\r\n",ptr, target_frame_ptr->val);
    uart_sendline("        Before\r\n");
    dump_page_info();
    target_frame_ptr->used = FRAME_FREE;
    while(coalesce(target_frame_ptr)==0); // merge buddy iteratively
    list_add(&target_frame_ptr->listhead, &frame_freelist[target_frame_ptr->val]);
    uart_sendline("        After\r\n");
    dump_page_info();
}

frame_t* release_redundant(frame_t *frame)
{
    frame->val -= 1;
    frame_t *buddyptr = get_buddy(frame);
    buddyptr->val = frame->val;
    list_add(&buddyptr->listhead, &frame_freelist[buddyptr->val]);
    return frame;
}

frame_t* get_buddy(frame_t *frame)
{
    // XOR(idx, order)
    return &frame_array[frame->idx ^ (1 << frame->val)];
}

int coalesce(frame_t *frame_ptr)
{
    frame_t *buddy = get_buddy(frame_ptr);
    // frame is the boundary
    if (frame_ptr->val == FRAME_IDX_FINAL)
        return -1;

    // Order must be the same: 2**i + 2**i = 2**(i+1)
    if (frame_ptr->val != buddy->val)
        return -1;

    // buddy is in used
    if (buddy->used == FRAME_ALLOCATED)
        return -1;

    list_del_entry((struct list_head *)buddy);
    frame_ptr->val += 1;
    uart_sendline("    coalesce detected, merging 0x%x, 0x%x, -> val = %d\r\n", frame_ptr->idx, buddy->idx, frame_ptr->val);
    return 0;
}

void dump_page_info(){
    unsigned int exp2 = 1;
    uart_sendline("        ----------------- [  Number of Available Page Blocks  ] -----------------\r\n        | ");
    for (int i = FRAME_IDX_0; i <= FRAME_IDX_FINAL; i++)
    {
        uart_sendline("%4dKB(%1d) ", 4*exp2, i);
        exp2 *= 2;
    }
    uart_sendline("|\r\n        | ");
    for (int i = FRAME_IDX_0; i <= FRAME_IDX_FINAL; i++)
        uart_sendline("     %4d ", list_size(&frame_freelist[i]));
    uart_sendline("|\r\n");
}

void dump_cache_info()
{
    unsigned int exp2 = 1;
    uart_sendline("    -- [  Number of Available Cache Blocks ] --\r\n    | ");
    for (int i = CACHE_IDX_0; i <= CACHE_IDX_FINAL; i++)
    {
        uart_sendline("%4dB(%1d) ", 32*exp2, i);
        exp2 *= 2;
    }
    uart_sendline("|\r\n    | ");
    for (int i = CACHE_IDX_0; i <= CACHE_IDX_FINAL; i++)
        uart_sendline("   %5d ", list_size(&cache_list[i]));
    uart_sendline("|\r\n");
}

void page2caches(int order)
{
    //make caches of the order from a page
    char *page = page_malloc(PAGESIZE);
    frame_t *pageframe_ptr = &frame_array[((unsigned long long)page - BUDDY_MEMORY_BASE) >> 12];
    pageframe_ptr->cache_order = order;

    // split page into a lot of caches and push them into cache_list
    int cachesize = (32 << order);
    for (int i = 0; i < PAGESIZE; i += cachesize)
    {
        list_head_t *c = (list_head_t *)(page + i);
        list_add(c, &cache_list[order]);
    }
}

void* cache_malloc(unsigned int size)
{
    uart_sendline("[+] Allocate cache - size : %d(0x%x)\r\n", size, size);
    uart_sendline("    Before\r\n");
    dump_cache_info();
    int order;
    for (int i = CACHE_IDX_0; i <= CACHE_IDX_FINAL; i++)
    {
        if (size <= (32 << i)) { order = i; break; }
    }

    if (list_empty(&cache_list[order]))
    {
        page2caches(order);
    }

    list_head_t* r = cache_list[order].next;
    list_del_entry(r);
    uart_sendline("    physical address : 0x%x\n", r);
    uart_sendline("    After\r\n");
    dump_cache_info();
    return r;
}

void cache_free(void *ptr)
{
    list_head_t *c = (list_head_t *)ptr;
    frame_t *pageframe_ptr = &frame_array[((unsigned long long)ptr - BUDDY_MEMORY_BASE) >> 12];
    uart_sendline("[+] Free cache: 0x%x, val = %d\r\n",ptr, pageframe_ptr->cache_order);
    uart_sendline("    Before\r\n");
    dump_cache_info();
    list_add(c, &cache_list[pageframe_ptr->cache_order]);
    uart_sendline("    After\r\n");
    dump_cache_info();
}

void *kmalloc(unsigned int size)
{
    uart_sendline("\n\n");
    uart_sendline("================================\r\n");
    uart_sendline("[+] Request kmalloc size: %d\r\n", size);
    uart_sendline("================================\r\n");
    //For page
    if (size > (32 << CACHE_IDX_FINAL))
    {
        void *r = page_malloc(size);
        return r;
    }
    void *r = cache_malloc(size);
    return r;
}

void kfree(void *ptr)
{
    uart_sendline("\n\n");
    uart_sendline("==========================\r\n");
    uart_sendline("[+] Request kfree 0x%x\r\n", ptr);
    uart_sendline("==========================\r\n");
    //For page
    if ((unsigned long long)ptr % PAGESIZE == 0 && frame_array[((unsigned long long)ptr - BUDDY_MEMORY_BASE) >> 12].cache_order == CACHE_NONE)
    {
        page_free(ptr);
        return;
    }
    cache_free(ptr);
}
