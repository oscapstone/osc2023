#include "mm.h"
#include "list.h"
#include "uart.h"
#include "exception.h"
#include "dtb.h"

extern char  __heap_top;
static char* htop_ptr = &__heap_top;
extern char  __start;
extern char  __end;
extern char  __stack_top;
extern char* dtb_ptr;

extern void* CPIO_DEFAULT_START;
extern void* CPIO_DEFAULT_END;

// ------ Lab2 ------
void* simple_malloc(unsigned int size) {
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

void simple_free(void* ptr) {
    // TBD
}

// ------ Lab4 ------
static frame_t*           frame_array;                    // store memory's statement and page's corresponding index
static list_head_t        frame_freelist[FRAME_MAX_IDX];  // store available block for page
static list_head_t        cache_list[CACHE_MAX_IDX];      // store available block for cache

void init_allocator()
{
    // Declare Frame Array through the heap, because the heap is responsible for managing the dynamic allocation of memory
    frame_array = simple_malloc(BUDDY_MEMORY_PAGE_COUNT * sizeof(frame_t));

    // Init frame_array from 0 ~ 0x3C000 ( spec )
    for (int i = 0; i < BUDDY_MEMORY_PAGE_COUNT; i++)
    {

        if (i % (1 << FRAME_IDX_FINAL) == 0)
        { 
            // Set the size of each frame to the largest block ( 2^6 x 4kb ), 
            // val is set to order 6 ( 2^6 x 4kb )
            frame_array[i].val = FRAME_IDX_FINAL; 
            // The idx’th frame is free, but it belongs to a larger contiguous memory block. 
            frame_array[i].used = FRAME_FREE; 
        }
    }

    // Init frame freelist from order 0 ~ 6 ( 2^val x 4kb )
    // Frame_freelist[i] can be used to manage free frames of different sizes. 
    // For example, frame_freelist[0] is used to manage free frames of size 2^0 x 4KB = 4KB, 
    // frame_freelist[1] is used to manage free frames of size 2^1 x 4KB = 8KB, and so on.
    for (int i = FRAME_IDX_0; i <= FRAME_IDX_FINAL; i++)
    {
        INIT_LIST_HEAD(&frame_freelist[i]);
    }

    // Init cache list from order 0 ~ 6 ( 2^val )
    // Cache_list[i] can be used to manage and track slab caches of different sizes. 
    // For example, cache_list[0] is used to manage a slab cache with a size of 2^0, 
    // cache_list[1] is used to manage a slab cache with a size of 2^1, and so on.
    for (int i = CACHE_IDX_0; i<= CACHE_IDX_FINAL; i++)
    {
        INIT_LIST_HEAD(&cache_list[i]);
    }


    for (int i = 0; i < BUDDY_MEMORY_PAGE_COUNT; i++)
    {
        // Init listhead for each frame
        INIT_LIST_HEAD(&frame_array[i].listhead);
        // Set index value for each page.
        frame_array[i].idx = i;
        // Set the cache_order of each page to CACHE_NONE ( page is not currently allocated )
        frame_array[i].cache_order = CACHE_NONE;

        // Add init frame (FRAME_IDX_FINAL) into freelist
        // Check if the current page index is a multiple of freelist. 
        // If yes, add the page to freelist.
        if (i % (1 << FRAME_IDX_FINAL) == 0)
        {
            list_add(&frame_array[i].listhead, &frame_freelist[FRAME_IDX_FINAL]);
        }
    }
    
    /* Startup reserving the following region:
    1. Spin tables for multicore boot (0x0000 - 0x1000)
    2. Devicetree (Optional, if you have implement it)
    3. Kernel image in the physical memory
    4. Your simple allocator (startup allocator) (Stack + Heap in my case)
    5. Initramfs
    */
    uart_printf("\r\n* Startup Allocation *\r\n");
    uart_printf("buddy system: usable memory region: 0x%x ~ 0x%x\n", BUDDY_MEMORY_BASE, BUDDY_MEMORY_BASE + BUDDY_MEMORY_PAGE_COUNT * PAGESIZE);
    
    
    dtb_find_and_store_reserved_memory(); // find spin tables in dtb
    uart_printf("\033[32m[Reserve for Kernel image]\n\033[0m");
    memory_reserve((unsigned long long)&__start, (unsigned long long)&__end); // kernel
    uart_printf("\033[32m[Reserve for Simple allocator]\n\033[0m");
    memory_reserve((unsigned long long)&__heap_top, (unsigned long long)&__stack_top);  // heap & stack -> simple allocator
    uart_printf("\033[32m[Reserve for Initramfs]\n\033[0m");
    memory_reserve((unsigned long long)CPIO_DEFAULT_START, (unsigned long long)CPIO_DEFAULT_END);
}

void* page_malloc(unsigned int size)
{
    uart_printf("    [+] Allocate page - size : %d(0x%x)\r\n", size, size);
    uart_printf("        Before\r\n");
    dump_page_info();

    int val =0;
    
    // Find the corresponding Page size
    // Turn size into minimum 2^val x 4KB
    for (int i = FRAME_IDX_0; i <= FRAME_IDX_FINAL; i++)
    {

        if (size <= (PAGESIZE << i))
        {
            val = i;
            uart_printf("        block size = 0x%x\n", PAGESIZE << i);
            break;
        }

        if ( i == FRAME_IDX_FINAL)
        {
            uart_printf("[!] request size exceeded for page_malloc!!!!\r\n");
            return (void*)0;
        }

    }

    // Find the smallest larger frame in freelist
    int target_val;
    for (target_val = val; target_val <= FRAME_IDX_FINAL; target_val++)
    {
        // Freelist does not have 2^i order frame, going for next order
        if (!list_empty(&frame_freelist[target_val]))
            break;
    }
    if (target_val > FRAME_IDX_FINAL)
    {
        uart_printf("[!] No available frame in freelist, page_malloc ERROR!!!!\r\n");
        return (void*)0;
    }


    // Get an available frame from the target list (freelist[target_val]), 
    // and store its pointer in the target_frame_ptr variable.
    frame_t *target_frame_ptr = (frame_t*)frame_freelist[target_val].next;
    // Removes the selected memory block from the free list, 
    // indicating that this block will be allocated. 
    list_del_entry((struct list_head *)target_frame_ptr);

    // Release redundant memory block to separate into pieces
    // If the fetched memory block is larger than actually needed, 
    // it is split into smaller blocks until the required size is met. 
    // The loop decrements from target_val (obtained block size) to val (actual required size).
    for (int j = target_val; j > val; j--) // ex: 10000 -> 01111
    {
        release_redundant(target_frame_ptr);
    }

    // Allocate it
    target_frame_ptr->used = FRAME_ALLOCATED;
    uart_printf("        physical address : 0x%x\n", BUDDY_MEMORY_BASE + (PAGESIZE*(target_frame_ptr->idx)));
    uart_printf("        After\r\n");
    dump_page_info();

    return (void *) BUDDY_MEMORY_BASE + (PAGESIZE * (target_frame_ptr->idx));
}

void page_free(void* ptr)
{
    // Calculate the position of the frame_t structure corresponding to the memory page 
    // that needs to be released in frame_array, and store its pointer in the target_frame_ptr variable.
    // Note. ">>12", which is to convert address offset to page frame index (2^12 = 4KB) .
    frame_t *target_frame_ptr = &frame_array[((unsigned long long)ptr - BUDDY_MEMORY_BASE) >> 12]; // PAGESIZE * Available Region -> 0x1000 * 0x10000000 // SPEC #1, #2
    uart_printf("    [+] Free page: 0x%x, val = %d\r\n",ptr, target_frame_ptr->val);
    uart_printf("        Before\r\n");
    dump_page_info();

    // Set the state of the target frame as free (FRAME_FREE).
    target_frame_ptr->used = FRAME_FREE;

    // Merge buddy iteratively, 
    // If the merge is successful, keep trying to merge the next buddy until no further merges are possible.
    while(coalesce(target_frame_ptr)==0); 

    // Add the frame corresponding to the released memory page to the free list (freelist) of the corresponding order, 
    // indicating that it is now available.
    list_add(&target_frame_ptr->listhead, &frame_freelist[target_frame_ptr->val]);
    uart_printf("        After\r\n");
    dump_page_info();
}

frame_t* release_redundant(frame_t *frame)
{
    // Order -1 -> add its buddy to free list (frame itself will be used in master function)
    frame->val -= 1;
    // Call the get_buddy function to find the buddy (buddy) of the current memory block, 
    // and store its pointer in the buddyptr variable.

    frame_t *buddyptr = get_buddy(frame);
    // Sets the Buddy's order (levels of size etc.) to be the same as the current memory block, 
    // since they are all split into smaller blocks of the same size.
    buddyptr->val = frame->val;

    // Add buddy to the freelist corresponding to the order, indicating that it is now available.
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
    uart_printf("    coalesce detected, merging 0x%x, 0x%x, -> val = %d\r\n", frame_ptr->idx, buddy->idx, frame_ptr->val);
    return 0;
}

void dump_page_info(){
    unsigned int exp2 = 1;
    uart_printf("        ----------------- [  Number of Available Page Blocks  ] -----------------\r\n        | ");
    for (int i = FRAME_IDX_0; i <= FRAME_IDX_FINAL; i++)
    {
        uart_printf("%4dKB(%1d) ", 4*exp2, i);
        exp2 *= 2;
    }
    uart_printf("|\r\n        | ");
    for (int i = FRAME_IDX_0; i <= FRAME_IDX_FINAL; i++)
        uart_printf("     %4d ", list_size(&frame_freelist[i]));
    uart_printf("|\r\n");
}

void dump_cache_info()
{
    unsigned int exp2 = 1;
    uart_printf("    -- [  Number of Available Cache Blocks ] --\r\n    | ");
    for (int i = CACHE_IDX_0; i <= CACHE_IDX_FINAL; i++)
    {
        uart_printf("%4dB(%1d) ", 32*exp2, i);
        exp2 *= 2;
    }
    uart_printf("|\r\n    | ");
    for (int i = CACHE_IDX_0; i <= CACHE_IDX_FINAL; i++)
        uart_printf("   %5d ", list_size(&cache_list[i]));
    uart_printf("|\r\n");
}

void page2caches(int order)
{
    // make caches from a smallest-size page
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
    uart_printf("[+] Allocate cache - size : %d(0x%x)\r\n", size, size);
    uart_printf("    Before\r\n");
    dump_cache_info();

    // Turn size into cache order: 2^order x 32B
    int order = 0;
    for (int i = CACHE_IDX_0; i <= CACHE_IDX_FINAL; i++)
    {
        if (size <= (32 << i)) { order = i; break; }
    }

    // If no available cache in list, assign one page for it
    if (list_empty(&cache_list[order]))
    {
        page2caches(order);
    }

    list_head_t* r = cache_list[order].next;
    list_del_entry(r);
    uart_printf("    physical address : 0x%x\n", r);
    uart_printf("    After\r\n");
    dump_cache_info();
    return r;
}

void cache_free(void *ptr)
{
    list_head_t *c = (list_head_t *)ptr;
    // Calculate the corresponding page frame pointer pageframe_ptr according to the passed pointer ptr. 
    // Here, ((unsigned long long)ptr - BUDDY_MEMORY_BASE) >> 12 calculates the page index where ptr is located, 
    // and then finds the corresponding page frame in frame_array.
    frame_t *pageframe_ptr = &frame_array[((unsigned long long)ptr - BUDDY_MEMORY_BASE) >> 12];
    uart_printf("[+] Free cache: 0x%x, val = %d\r\n",ptr, pageframe_ptr->cache_order);
    uart_printf("    Before\r\n");
    dump_cache_info();
    list_add(c, &cache_list[pageframe_ptr->cache_order]);
    uart_printf("    After\r\n");
    dump_cache_info();
}

void *kmalloc(unsigned int size)
{
    uart_printf("\n\n");
    uart_printf("\033[32m================================\n\033[0m\r");
    uart_printf("\033[32m[+] Request kmalloc size: %d\n\033[0m\r", size);
    uart_printf("\033[32m================================\n\033[0m\r\n");
    // If size is larger than cache size, go for page
    if (size > (32 << CACHE_IDX_FINAL))
    {
        void *r = page_malloc(size);
        return r;
    }
    // Go for cache
    void *r = cache_malloc(size);
    return r;
}

void kfree(void *ptr)
{

    uart_printf("\n\n");
    uart_printf("\033[32m==========================\n\033[0m\r");
    uart_printf("\033[32m[+] Request kfree 0x%x\033[0m\r\n", ptr);
    uart_printf("\033[32m==========================\033[0m\r\n");
    // If no cache assigned, go for page
    if ((unsigned long long)ptr % PAGESIZE == 0 && frame_array[((unsigned long long)ptr - BUDDY_MEMORY_BASE) >> 12].cache_order == CACHE_NONE)
    {
        page_free(ptr);
        return;
    }
    // go for cache
    cache_free(ptr);
}

void memory_reserve(unsigned long long start, unsigned long long end)
{
    start -= start % PAGESIZE; // floor (align 0x1000)
    end = end % PAGESIZE ? end + PAGESIZE - (end % PAGESIZE) : end; // ceiling (align 0x1000)

    uart_printf("Reserved Memory: ");
    uart_printf("start 0x%x ~ ", start);
    uart_printf("end 0x%x\r\n",end);

    // delete page from free list
    for (int order = FRAME_IDX_FINAL; order >= 0; order--)
    {
        list_head_t *pos;
        list_for_each(pos, &frame_freelist[order])
        {
            unsigned long long pagestart = ((frame_t *)pos)->idx * PAGESIZE + BUDDY_MEMORY_BASE;
            unsigned long long pageend = pagestart + (PAGESIZE << order);

            // Start at the largest level (FRAME_IDX_FINAL) and walk down through the page frames of each level, 
            // checking whether each page intersects with the reserved memory range.
            if (start <= pagestart && end >= pageend) // if page all in reserved memory -> delete it from freelist
            {
                // If the page lies entirely within the reserved memory range, 
                // remove it from the free list and mark it as allocated
                ((frame_t *)pos)->used = FRAME_ALLOCATED;
                uart_printf("    [!] Reserved page in 0x%x - 0x%x\n", pagestart, pageend);
                uart_printf("        Before\n");
                dump_page_info();
                list_del_entry(pos);
                uart_printf("        Remove usable block for reserved memory: order %d\r\n", order);
                uart_printf("        After\n");
                dump_page_info();
            }
            else if (start >= pageend || end <= pagestart) // no intersection
            {
                continue;
            }
            else // partial intersection, separate the page into smaller size.
            {
                list_del_entry(pos);
                list_head_t *temppos = pos -> prev;
                list_add(&release_redundant((frame_t *)pos)->listhead, &frame_freelist[order - 1]);
                pos = temppos;
            }
        }
    }
}