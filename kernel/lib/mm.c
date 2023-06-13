#include "uint.h"
#include "uart.h"
#include "mm.h"
#include "malloc.h"
#include "dtb.h"

extern char __text_start;
extern char __heap_start;
extern char __startup_allocator_start;
extern char __startup_allocator_end;

static uint64_t pf_base;
static uint64_t pf_end;
static uint32_t pf_count;

frame_entry *pf_entries;
list_head_t pf_freelists[MAX_ORDER];

uint32_t chunk_slot_size[] = {
    16, 32, 64, 128, 256, 512, 1024, 2048, 4096
};
chunk_slot_entry *cs_entries;
list_head_t cs_freelists[9];

void page_frame_allocator_init(void *start, void *end)
{
    pf_base = (uint64_t)start, pf_end = (uint64_t)end;
    pf_count = (pf_end - pf_base) / PAGE_SIZE;
    pf_entries = smalloc(pf_count * sizeof(frame_entry));

    for (int i = 0; i < pf_count; ++i)
        pf_entries[i].order = 0, pf_entries[i].status = FREE;
}

void page_frame_init_merge()
{
    for (int i = 0; i < MAX_ORDER - 1; ++i)
    {
        int f1 = 0, f2 = 0;
        for (;;)
        {
            f2 = f1 ^ (1 << i);

            if (f2 >= pf_count)
                break;

            if (pf_entries[f1].status == FREE && pf_entries[f2].status == FREE &&
                pf_entries[f1].order == i && pf_entries[f2].order == i)
                ++pf_entries[f1].order;
            
            f1 += (1 << (i + 1));

            if (f1 >= pf_count)
                break;
        }
    }

    for (int i = 0; i < MAX_ORDER; ++i)
        INIT_LIST_HEAD(&pf_freelists[i]);

    for (int i = 0, odr = 0; i < pf_count; i += (1 << odr))
    {
        odr = pf_entries[i].order;
        if (pf_entries[i].status == FREE)
        {
            frame_entry_list_head *tmp;
            tmp = idx2address(i);
            list_add_tail(&tmp->listhead, &pf_freelists[odr]);
#ifdef DEMO
            uart_printf("%d added to %d'th order list\n", i, odr);
#endif
        }
    }
}

void *page_frame_allocation(uint32_t page_num)
{

    if (page_num == 0)
        return (void *)0;

    int order = log2n(page_num);
    int alloc_page_order = order;

    while (list_empty(&pf_freelists[alloc_page_order]))
        ++alloc_page_order;

    if (alloc_page_order == MAX_ORDER)
    {
        uart_printf("No page available !!! \n");
        return (void *)0;
    }

#ifdef DEMO
    uart_printf("ideal page order: %d\n", order);
    uart_printf("alloc page order: %d\n", alloc_page_order);
#endif

    frame_entry_list_head *felhp = (frame_entry_list_head *)pf_freelists[alloc_page_order].next;
    int idx = address2idx(felhp);

    list_del_entry(&felhp->listhead);

#ifdef DEMO
    uart_printf("alloc page index: %d\n", idx);
#endif

    // redundant block release
    while (alloc_page_order > order)
    {
        int bd_idx = idx ^ (1 << (--alloc_page_order));
        pf_entries[bd_idx].order = alloc_page_order;
        pf_entries[bd_idx].status = FREE;

#ifdef DEMO
    uart_printf("released redundant page index: %d\n", bd_idx);
    uart_printf("released redundant page order: %d\n", alloc_page_order);
#endif

        frame_entry_list_head *bd_felhp = idx2address(bd_idx);
        list_add(&bd_felhp->listhead, &pf_freelists[alloc_page_order]);
    }

    pf_entries[idx].order = order;
    pf_entries[idx].status = ALLOCATED;

#ifdef DEMO
    uart_printf("page is allocated at: %x\n", felhp);
    uart_printf("--------------------------------------------\n");
#endif
    return (void*)felhp;
}

void page_frame_free(void *address)
{
#ifdef DEMO
    uart_printf("--------------------------------------------\n");
    uart_printf("freeing: %x\n", address);
#endif
    frame_entry_list_head *page = (frame_entry_list_head *)address;
    int idx = address2idx(page);
    int order = pf_entries[idx].order;
    pf_entries[idx].status = FREE;
    list_add(&page->listhead, &pf_freelists[order]);

    int bd_idx = idx ^ (1 << order);
#ifdef DEMO
    uart_printf("free page order: %d\n", order);
    uart_printf("free page index: %d\n", idx);
#endif

    // Merge
    while (order < MAX_ORDER - 1 && pf_entries[bd_idx].status == FREE && pf_entries[bd_idx].order == order)
    {
#ifdef DEMO
    uart_printf("target page index: %d\n", idx);
    uart_printf("buddy page index: %d\n", bd_idx);
    uart_printf("page order: %d\n", order);
#endif
        frame_entry_list_head *tmp = (frame_entry_list_head *)(idx2address(idx));
        list_del_entry(&tmp->listhead);
        tmp = (frame_entry_list_head *)(idx2address(bd_idx));
        list_del_entry(&tmp->listhead);

        idx &= bd_idx;
        pf_entries[idx].order = ++order; 
        tmp = (frame_entry_list_head *)(idx2address(idx));
        list_add(&tmp->listhead, &pf_freelists[order]);
        bd_idx = idx ^ (1 << order);
    }
#ifdef DEMO
    uart_printf("--------------------------------------------\n");
#endif
}

void chunk_slot_allocator_init()
{
    cs_entries = smalloc(pf_count * sizeof(chunk_slot_entry));
    for (int i = 0; i < pf_count; ++i)
        cs_entries[i].status = FREE;
}

void chunk_slot_listhead_init()
{
    for (int i = 0; i < 9; ++i)
        INIT_LIST_HEAD(&cs_freelists[i]);
}

void *chunk_slot_allocation(uint32_t size)
{
    int size_idx = find_fit_chunk_slot(size);
#ifdef DEMO
        uart_printf("alloc chunk slot size: %d\n", chunk_slot_size[size_idx]);
#endif
    chunk_slot_list_head *cslh;

    if (list_empty(&cs_freelists[size_idx]))
    {
        void *page_address = page_frame_allocation(1);
        int page_idx = address2idx(page_address);

#ifdef DEMO
        uart_printf("Need page allocation !!! \n");
        uart_printf("Split page %d into chunks of size %d\n", page_idx, chunk_slot_size[size_idx]);
#endif

        cs_entries[page_idx].size = size_idx;
        cs_entries[page_idx].status = ALLOCATED;

        for (int i = 0; i + chunk_slot_size[size_idx] <= PAGE_SIZE; i += chunk_slot_size[size_idx])
        {
            cslh = (chunk_slot_list_head *)((char *)page_address + i);
            list_add_tail(&cslh->listhead, &cs_freelists[size_idx]);
        }
    }

    cslh = (chunk_slot_list_head *)cs_freelists[size_idx].next;
    list_del_entry(&cslh->listhead);

#ifdef DEMO
    uart_printf("chunk is allocated at: %x\n", cslh);
    uart_printf("--------------------------------------------\n");
#endif
    return cslh;
}

void chunk_slot_free(void *address)
{
    int page_idx = address2idx(address);
    chunk_slot_list_head *cslh = (chunk_slot_list_head *)address;

    int size = cs_entries[page_idx].size;
    list_add(&cslh->listhead, &cs_freelists[size]);

#ifdef DEMO
        uart_printf("Freeing %x chunk of size %d in page %d\n", address, chunk_slot_size[size], page_idx);
#endif
}

void memory_reserve(void *start, void *end)
{
    start = (void *)((uint64_t) start / PAGE_SIZE * PAGE_SIZE);
    end = (void *)(((uint64_t) end + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE);

    uart_printf("start: %x, end: %x has been reserved\n", (uint64_t)start, (uint64_t)end);

    for (void *tmp = start; tmp < end; tmp = (void *)((uint64_t) tmp + PAGE_SIZE))
        pf_entries[address2idx(tmp)].status = ALLOCATED;
}

void memory_init()
{
    page_frame_allocator_init((void *)0, (void *)0x3c000000);
    chunk_slot_allocator_init();

    memory_reserve((void *)0, (void *)0x1000);
    memory_reserve(&__text_start, &__heap_start);
    memory_reserve(cpio_start, cpio_end);
    memory_reserve(&__startup_allocator_start, &__startup_allocator_end);

    page_frame_init_merge();
    chunk_slot_listhead_init();
}

void *malloc(uint32_t size)
{
    if (size < PAGE_SIZE)
    {
#ifdef DEMO
        uart_printf("--------------------------------------------\n");
        uart_printf("size: %d\n", size);
#endif
        return chunk_slot_allocation(size);
    }
    else
    {
        uint32_t pn = (size + PAGE_SIZE - 1) / PAGE_SIZE;
#ifdef DEMO
        uart_printf("--------------------------------------------\n");
        uart_printf("size: %d, page num: %d\n", size, pn);
#endif
        return page_frame_allocation(pn);
    }
}

void page_frame_allocator_test()
{
    char *pages[10];

    pages[0] = malloc(1*PAGE_SIZE + 123);
    pages[1] = malloc(1*PAGE_SIZE);
    pages[2] = malloc(3*PAGE_SIZE + 321);
    // release redundant
    pages[3] = malloc(1*PAGE_SIZE + 31);
    //
    pages[4] = malloc(1*PAGE_SIZE + 21);
    pages[5] = malloc(1*PAGE_SIZE);

    page_frame_free(pages[2]);
    pages[6] = malloc(1*PAGE_SIZE);
    pages[7] = malloc(1*PAGE_SIZE + 333);
    pages[8] = malloc(1*PAGE_SIZE);

    // Merge blocks
    page_frame_free(pages[6]);
    page_frame_free(pages[7]);
    page_frame_free(pages[8]);

    // free all
    page_frame_free(pages[0]);
    page_frame_free(pages[1]);
    page_frame_free(pages[3]);
    page_frame_free(pages[4]);
    page_frame_free(pages[5]);
}

void chunk_slot_allocator_test()
{
    char *chunk[100];
    
    int tmp = PAGE_SIZE / 512;

    for (int i = 0; i <= tmp; ++i)
        chunk[i] = malloc(0x101);
    
    chunk[tmp + 1] = malloc(0x11);
    chunk[tmp + 2] = malloc(0x15);

    chunk[tmp + 3] = malloc(0x25);
    chunk[tmp + 4] = malloc(0x35);

    for (int i = 0; i <= tmp; ++i)
        chunk_slot_free(chunk[i]);

    chunk_slot_free(chunk[tmp + 1]);
    chunk_slot_free(chunk[tmp + 2]);
    chunk_slot_free(chunk[tmp + 3]);
    chunk_slot_free(chunk[tmp + 4]);
}

/* Utility functions */

uint32_t log2n(uint32_t x)
{
    return (x > 1) ? 1 + log2n(x / 2) : 0;
}

uint32_t address2idx(void *address)
{
    return ((uint64_t)address - pf_base) / PAGE_SIZE;
}

void *idx2address(uint32_t idx)
{
    return (void *)(pf_base + idx * PAGE_SIZE);
}

int find_fit_chunk_slot(uint32_t size)
{
    for (uint32_t i = 0 ; i < 9; ++i)
        if (chunk_slot_size[i] >= size)
            return i;
    return -1;
}