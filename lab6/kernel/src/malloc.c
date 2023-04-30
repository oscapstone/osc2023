#include "malloc.h"
#include "cpio.h"
#include "dtb.h"
#include "exception.h"
#include "mmu.h"
#include "uart1.h"

extern char _heap_start;
extern char _end;
static char *simple_top = &_heap_start;
static char *kernel_end = &_end;

// simple_malloc
void *simple_malloc(unsigned int size)
{
    char *r = simple_top + 0x10;
    size = 0x10 + size - size % 0x10;
    *(unsigned int *)(r - 0x8) = size;
    simple_top += size;
    return r;
}

static frame_t* framearray;
static list_head_t freelist[MAXORDER + 1];       // 4K * (idx**ORDER) (for every 4K) (page)
static list_head_t cachelist[MAXCACHEORDER + 1]; // 32, 64, 128, 256, 512  (for every 32bytes)

void init_allocator()
{
    framearray = simple_malloc(BUDDYSYSTEM_PAGE_COUNT * sizeof(frame_t));

    for (int i = 0; i < BUDDYSYSTEM_PAGE_COUNT; i++)
    {
        if (i % (1 << MAXORDER) == 0)
        {
            framearray[i].isused = 0;
            framearray[i].val = 6;
        }
    }

    //init frame freelist
    for (int i = 0; i <= MAXORDER; i++)
    {
        INIT_LIST_HEAD(&freelist[i]);
    }

    for (int i = 0; i < BUDDYSYSTEM_PAGE_COUNT; i++)
    {
        //init listhead for each frame
        INIT_LIST_HEAD(&framearray[i].listhead);
        framearray[i].idx = i;
        framearray[i].cacheorder = -1;

        //add init frame into freelist
        if (i % (1 << MAXORDER) == 0)
        {
            list_add(&framearray[i].listhead, &freelist[MAXORDER]);
        }
    }

    //init cachelist
    for (int i = 0; i <= MAXCACHEORDER; i++)
    {
        INIT_LIST_HEAD(&cachelist[i]);
    }


    /* should reserve these memory region
    Spin tables for multicore boot (0x0000 - 0x1000)
    Kernel image in the physical memory
    Initramfs
    Devicetree (Optional, if you have implement it)
    Your simple allocator (startup allocator)  
    stack
   */
    dtb_find_and_store_reserved_memory(); // spin tables can be find
    memory_reserve(PHYS_TO_VIRT(0x1000), PHYS_TO_VIRT(0x5000)); // // PGD's page frame at 0x1000 // PUD's page frame at 0x2000 PMD 0x3000-0x5000
    memory_reserve(PHYS_TO_VIRT(0x80000), (unsigned long long)kernel_end);              //kernel
    memory_reserve((unsigned long long)&_heap_start, (unsigned long long)simple_top); //simple
    memory_reserve((unsigned long long)CPIO_DEFAULT_START, (unsigned long long)CPIO_DEFAULT_END);
    memory_reserve(PHYS_TO_VIRT(0x2c000000), PHYS_TO_VIRT(0x3c000000)); //0x2c000000L - 0x3c000000L (stack)
}

//smallest 4K
void *allocpage(unsigned int size)
{
    // get real val size
    //int allocsize;
    int val;
    for (int i = 0; i <= MAXORDER; i++)
    {

        if (size <= (0x1000 << i))
        {
            val = i;
            break;
        }

        if (i == MAXORDER)
        {
            uart_puts("Too large size for kmalloc!!!!\r\n");
            while (1);
            return simple_malloc(size);
        }
    }

    // find the smallest larger frame in freelist
    int target_list_val;
    for (target_list_val = val; target_list_val <= MAXORDER; target_list_val++)
    {
        if (!list_empty(&freelist[target_list_val]))
            break;
    }

    if (target_list_val > MAXORDER)
    {
        uart_puts("kmalloc ERROR (all lists are empty?)!!!!\r\n");
        while (1);
        return simple_malloc(size);
    }

    //get the frame
    frame_t *target_frame_ptr = (frame_t *)freelist[target_list_val].next;
    list_del_entry((struct list_head *)target_frame_ptr);

    // Release redundant memory block
    for (int j = target_list_val; j > val; j--)
    {
        release_redundant(target_frame_ptr);
    }
    target_frame_ptr->isused = 1;
    return (void *)BUDDYSYSTEM_START + (0x1000 * (target_frame_ptr->idx));
}

void freepage(void *ptr)
{
    frame_t *target_frame_ptr = &framearray[((unsigned long long)ptr - BUDDYSYSTEM_START) >> 12];

    target_frame_ptr->isused = 0;
    frame_t* temp;
    while ((temp = coalesce(target_frame_ptr)) != (frame_t *)-1)target_frame_ptr = temp;

    list_add(&target_frame_ptr->listhead, &freelist[target_frame_ptr->val]);
}

frame_t *release_redundant(frame_t *frame_ptr)
{
    frame_ptr->val -= 1;
    frame_t *buddyptr = get_buddy(frame_ptr);
    buddyptr->val = frame_ptr->val;
    buddyptr->isused = 0;
    list_add(&buddyptr->listhead, &freelist[buddyptr->val]);
    return frame_ptr;
}

frame_t *get_buddy(frame_t *frame)
{
    return &framearray[frame->idx ^ (1 << frame->val)];
}

//return 0  -> success
//return -1 -> cannot coalesce
frame_t* coalesce(frame_t *frame_ptr)
{
    frame_t *buddy = get_buddy(frame_ptr);

    // MAXORDER
    if (frame_ptr->val == MAXORDER)
        return (frame_t*)-1;

    // val not the same (there is some chunks in the buddy used)
    if (frame_ptr->val != buddy->val)
        return (frame_t *)-1;

    //buddy is used
    if (buddy->isused == 1)
        return (frame_t *)-1;

    list_del_entry((struct list_head *)buddy);
    frame_ptr->val += 1;
    buddy->val += 1;

    return buddy<frame_ptr?buddy:frame_ptr;
}

void *alloccache(unsigned int size)
{
    int order;
    for (int i = 0; i <= MAXCACHEORDER; i++)
    {
        if (size <= (32 << i))
        {
            order = i;
            break;
        }
    }
    if (list_empty(&cachelist[order]))
    {
        page2caches(order);
    }
    list_head_t *r = cachelist[order].next;
    list_del_entry(r);

    return r;
}

void page2caches(int order)
{
    //make caches of the order from a page
    char *page = allocpage(0x1000);
    frame_t *pageframe_ptr = &framearray[((unsigned long long)page - BUDDYSYSTEM_START) >> 12];
    pageframe_ptr->cacheorder = order;

    // split page into a lot of caches and push them into cachelist
    int cachesize = (32 << order);
    for (int i = 0; i < 0x1000; i += cachesize)
    {
        list_head_t *c = (list_head_t *)(page + i);
        list_add(c, &cachelist[order]);
    }
}

void freecache(void *ptr)
{
    list_head_t *c = (list_head_t *)ptr;
    frame_t *pageframe_ptr = &framearray[((unsigned long long)ptr - BUDDYSYSTEM_START) >> 12];
    list_add(c, &cachelist[pageframe_ptr->cacheorder]);
}

void *kmalloc(unsigned int size)
{
    lock();
    //For page
    if (size > (32 << MAXCACHEORDER))
    {
        void *r = allocpage(size);
        unlock();
        return r;
    }

    void *r = alloccache(size);

    //For cache
    unlock();
    return r;
}

void kfree(void *ptr)
{
    lock();
    //For page
    if ((unsigned long long)ptr % 0x1000 == 0 && framearray[((unsigned long long)ptr - BUDDYSYSTEM_START) >> 12].cacheorder == -1)
    {
        freepage(ptr);
        unlock();
        return;
    }

    //For cache
    freecache(ptr);
    unlock();
}

void memory_reserve(unsigned long long start, unsigned long long end)
{
    uart_sendline("reserve -> start : 0x%x end : 0x%x\r\n", start, end);
    start -= start % 0x1000; // floor (align 0x1000)
    end = end % 0x1000 ? end + 0x1000 - (end % 0x1000) : end; // ceiling (align 0x1000)

    //delete page from freelist
    for (int order = MAXORDER; order >= 0; order--)
    {
        list_head_t *pos;
        list_for_each(pos, &freelist[order])
        {
            unsigned long long pagestart = ((frame_t *)pos)->idx * 0x1000L + BUDDYSYSTEM_START;
            unsigned long long pageend = pagestart + (0x1000L << order);

            if (start <= pagestart && end >= pageend) // if page all in reserved memory -> delete it from freelist
            {
                ((frame_t *)pos)->isused = 1;
                list_del_entry(pos);
            }
            else if (start >= pageend || end <= pagestart) // no intersection
            {
                continue;
            }
            else // partial intersection (or reversed memory all in the page)
            {
                list_del_entry(pos);
                list_head_t *temppos = pos -> prev;
                list_add(&release_redundant((frame_t *)pos)->listhead, &freelist[order - 1]);
                pos = temppos;
            }
        }
    }
}
