#include "malloc.h"
#include "uart.h"
#include "list.h"
#include "type.h"
#include "memory.h"

extern char __text_start;
extern char __heap_start;
extern char __startup_allocator_start;
extern char __startup_allocator_end;
extern char *cpio_start;
extern char *cpio_end;


uint32_t chunk_slot_size[] = {
    16, 32, 64, 128, 256, 512, 1024, 2048, 4096
};

frame_entry * frame_entries;
chunk_entry * chunk_entries;

list_head_t frame_free_lists[MAX_ORDER];  // linked list array
list_head_t chunk_free_lists[9];


uint32_t f_count;
uint64_t mem_start;
uint64_t mem_end;


/* initialization */
void init_frames() {
    f_count = (mem_end - mem_start) / PAGE_SIZE;
    frame_entries = simple_malloc(f_count * sizeof(frame_entry));
    for (int i = 0; i < f_count; ++i) {
        frame_entries[i].order = 0;
        frame_entries[i].status = FREE;
    }
}

void init_chunks() {
    chunk_entries = simple_malloc(f_count * sizeof(chunk_entry));
    for (int i = 0; i < f_count; ++i) {
        chunk_entries[i].status = FREE;
    }
}

void memory_reserve(void * start, void * end) {
    start = (void *)((uint64_t) start / PAGE_SIZE * PAGE_SIZE);
    end = (void *)(((uint64_t) end + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE);

#ifdef DEMO
    uart_printf("start: %d, end: %d has been reserved\n", (uint64_t)start, (uint64_t)end);
#endif

    for (void *tmp = start; tmp < end; tmp = (void *)((uint64_t) tmp + PAGE_SIZE)) {
        frame_entries[address2idx(tmp)].status = ALLOCATED;
    }
}

void init_memory() {
    mem_start = (uint64_t)MEMORY_START;
    mem_end = (uint64_t)MEMORY_END;
    
    init_frames();
    init_chunks();

    memory_reserve((void *)0, (void *)0x1000);
    memory_reserve(&__text_start, &__heap_start);
    memory_reserve(cpio_start, cpio_end);
    memory_reserve(&__startup_allocator_start, &__startup_allocator_end);

    init_merge_frames();
    init_chunk_listhead();
}

void init_merge_frames() {
    for (int i = 0; i < MAX_ORDER - 1; ++i) {
        int f1 = 0, f2 = 0; // start with head
        for (;;)
        {
            f2 = f1 ^ (1 << i); // order i

            if (f2 >= f_count)
                break;

            if (frame_entries[f1].status == FREE && frame_entries[f2].status == FREE &&
                frame_entries[f1].order == i && frame_entries[f2].order == i)
                ++frame_entries[f1].order;
            
            f1 += (1 << (i + 1)); // skip merged frames (buddy and itself), which is 2 to the power of i + 1

            if (f1 >= f_count)
                break;
        }
    }

    for (int i = 0; i < MAX_ORDER; ++i) {
        INIT_LIST_HEAD(&frame_free_lists[i]);
    }

    for (int i = 0, order = 0; i < f_count; i += (1 << order)) {
        order = frame_entries[i].order;
        if (frame_entries[i].status == FREE) {
            frame_entry_list_head * tmp;
            tmp = idx2address(i); // directly give the first 16 bytes of frame to frame_entry_list_head
            list_add(&tmp->listhead, &frame_free_lists[order]);
#ifdef DEMO
            uart_printf("frame %d added to %d'th order list\n", i, order);
#endif
        }
        // the order of allocated frame is 0, so skip with i+=1
    }
}

void init_chunk_listhead()
{
    for (int i = 0; i < 9; ++i) {
        INIT_LIST_HEAD(&chunk_free_lists[i]);
    }
}

/* allocation */
void * allocate_frame(uint32_t page_num) { // page_num = the number of page needed
    if (page_num == 0) {
        return (void *)0;
    }

    int origin_order = log2n(page_num);
    int allocate_order = origin_order;
    while (allocate_order < MAX_ORDER && list_empty(&frame_free_lists[allocate_order])) {
        allocate_order++; // no free frame -> add order
    }

    if (allocate_order == MAX_ORDER) {
        uart_printf("No available page !\n");
        return (void *)0;
    }

#ifdef DEMO
    uart_printf("ideal page order: %d\n", origin_order);
    uart_printf("alloc page order: %d\n", allocate_order);
#endif

    frame_entry_list_head * ptr =  (frame_entry_list_head *)frame_free_lists[allocate_order].next;
    int idx = address2idx(ptr);

    frame_entries[idx].order = origin_order;
    frame_entries[idx].status = ALLOCATED;
    list_del_entry(&ptr->listhead);

#ifdef DEMO
    uart_printf("allocate page index: %d\n", idx);
#endif

    // release redundant frame
    while (allocate_order > origin_order) {
        allocate_order--;
        int cut_half_idx = idx ^ (1 << allocate_order);
        frame_entries[cut_half_idx].order = allocate_order;
        frame_entries[cut_half_idx].status = FREE;

#ifdef DEMO
        uart_printf("released redundant page index: %d\n", cut_half_idx);
        uart_printf("released redundant page order: %d\n", allocate_order);
#endif

        frame_entry_list_head * tmp;
        tmp = idx2address(cut_half_idx); // directly give the first 16 bytes of frame to frame_entry_list_head
        list_add(&tmp->listhead, &frame_free_lists[allocate_order]);
    }

#ifdef DEMO
    uart_printf("page is allocated at: %x\n", ptr);
    uart_printf("--------------------------------------------\n");
#endif

    return (void *)ptr;
}

void * allocate_chunk(uint32_t size) {
    int size_idx = find_fit_chunk_slot(size);
    int chunk_size = chunk_slot_size[size_idx];

#ifdef DEMO
    uart_printf("alloc chunk slot size: %d\n", chunk_size);
#endif

    chunk_entry_list_head * ptr;
    if (list_empty(&chunk_free_lists[size_idx])) { // no free chunk with the size
        void * page_address = allocate_frame(1);
        int page_idx = address2idx(page_address);
        chunk_entries[page_idx].size = size_idx;
        chunk_entries[page_idx].status = ALLOCATED;

        for (int i = 0; i + chunk_size <= PAGE_SIZE; i += chunk_size) {
            ptr = (chunk_entry_list_head *)((char *)page_address + i); // char = 1 byte
            list_add(&ptr->listhead, &chunk_free_lists[size_idx]);
        }

#ifdef DEMO
        uart_printf("Need page allocation !\n");
        uart_printf("Split page %d into chunks of size %d\n", page_idx, chunk_size);
#endif        
    }

    ptr = (chunk_entry_list_head *)chunk_free_lists[size_idx].next;
    list_del_entry(&ptr->listhead);


#ifdef DEMO
    uart_printf("chunk is allocated at: %x\n", ptr);
    uart_printf("--------------------------------------------\n");
#endif

    return (void *)ptr;
}

/* free */
void free_frame(void * address) {
    int idx = address2idx(address);
    int order = frame_entries[idx].order;
    frame_entries[idx].status = FREE;
    frame_entry_list_head *page = (frame_entry_list_head *)address;
    list_add(&page->listhead, &frame_free_lists[order]);

#ifdef DEMO
    uart_printf("free page order: %d\n", order);
    uart_printf("free page index: %d\n", idx);
#endif

    // merge
    int buddy_idx = idx ^ (1 << order);
    frame_entry_list_head * buddy_ptr1, * buddy_ptr2, * left_buddy_ptr;
    while (order < MAX_ORDER - 1 && frame_entries[buddy_idx].status == FREE && frame_entries[buddy_idx].order == order) {

#ifdef DEMO
    uart_printf("target page index: %d\n", idx);
    uart_printf("buddy page index: %d\n", buddy_idx);
    uart_printf("page order: %d\n", order);
#endif
        order++;

        // delete both left buddy and right buddy
        buddy_ptr1 = (frame_entry_list_head *)idx2address(buddy_idx);
        list_del_entry(&buddy_ptr1->listhead);
        buddy_ptr2 = (frame_entry_list_head *)idx2address(idx);
        list_del_entry(&buddy_ptr2->listhead);
        
        // add left buddy to list
        idx &= buddy_idx; // guarantee idx to be left buddy
        frame_entries[idx].order = order;
        left_buddy_ptr = (frame_entry_list_head *)idx2address(idx);
        list_add(&left_buddy_ptr->listhead, &frame_free_lists[order]);

        // find the buddy index of the next order
        buddy_idx = idx ^ (1 << order);
    }
    
#ifdef DEMO
    uart_printf("--------------------------------------------\n");
#endif
    
}

void free_chunk(void * address) {
    chunk_entry_list_head * ptr = (chunk_entry_list_head *)address;
    int page_idx = address2idx(address);
    int size_idx = chunk_entries[page_idx].size;
    list_add(&ptr->listhead, &chunk_free_lists[size_idx]);

#ifdef DEMO
    uart_printf("Freeing %x chunk of size %d in page %d\n", address, chunk_slot_size[size_idx], page_idx);
    uart_printf("--------------------------------------------\n");
#endif
}

/* malloc */
void * malloc(uint32_t size) {
    if (size < PAGE_SIZE) {
        // allocate chunk
        return allocate_chunk(size);

#ifdef DEMO
        uart_printf("malloc size: %d\n", size);
#endif
    }
    else {
        // allocate page
        uint32_t page_num = (size + PAGE_SIZE - 1) / PAGE_SIZE;

#ifdef DEMO
        uart_printf("malloc size: %d, page num: %d\n", size, page_num);
#endif
        return allocate_frame(page_num);
    }
}

/* utility */
uint32_t log2n(uint32_t x) {
    return (x > 1) ? 1 + log2n(x / 2) : 0;
}

uint32_t log2(uint32_t x) {
    uint32_t r;
    r = (x > 0xFFFF) << 4; x >>= r;
    uint32_t shift = (x > 0xFF) << 3;
    x >>= shift; r |= shift;
    shift = (x > 0xF) << 2;
    x >>= shift; r |= shift;
    shift = (x > 0x3) << 1;
    x >>= shift; r |= shift;
    return r | (x >> 1);
}

uint32_t address2idx(void *address) { // return frame index given frame or chunk address
    return ((uint64_t)address - mem_start) / PAGE_SIZE;
}

void * idx2address(uint32_t idx) { // return pointer of frame given index
    return (void *)(mem_start + idx * PAGE_SIZE);
}

int find_fit_chunk_slot(uint32_t size) {
    for (uint32_t i = 0 ; i < 9; ++i)
        if (chunk_slot_size[i] >= size)
            return i;
    return -1;
}

/* test */
void page_frame_allocator_test() {
    char *pages[10];

    uart_printf("Test 0\n");
    pages[0] = malloc(1*PAGE_SIZE + 123);
    uart_printf("Test 1\n");
    pages[1] = malloc(1*PAGE_SIZE);
    uart_printf("Test 2\n");
    pages[2] = malloc(3*PAGE_SIZE + 321);
    uart_printf("Test 3\n");
    pages[3] = malloc(1*PAGE_SIZE + 31);
    // release redundant
    uart_printf("Test 4\n");
    pages[4] = malloc(1*PAGE_SIZE + 21);
    uart_printf("Test 5\n");
    pages[5] = malloc(1*PAGE_SIZE);

    free_frame(pages[2]);
    uart_printf("Test 6\n");
    pages[6] = malloc(1*PAGE_SIZE);
    uart_printf("Test 7\n");
    pages[7] = malloc(1*PAGE_SIZE + 333);
    uart_printf("Test 8\n");
    pages[8] = malloc(1*PAGE_SIZE);

    // Merge blocks
    free_frame(pages[6]);
    free_frame(pages[7]);
    free_frame(pages[8]);

    // free all
    free_frame(pages[0]);
    free_frame(pages[1]);
    free_frame(pages[3]);
    free_frame(pages[4]);
    free_frame(pages[5]);
}

void chunk_slot_allocator_test() {
    char *chunk[100];
    
    int tmp = PAGE_SIZE / 512;

    // use a whole page
    for (int i = 0; i <= tmp; ++i)
        chunk[i] = malloc(0x101); // malloc 257 -> use 512
    
    // get another page
    // size 16
    chunk[tmp + 1] = malloc(0x11); // 17
    chunk[tmp + 2] = malloc(0x15); // 21
    // size 32
    chunk[tmp + 3] = malloc(0x25); // 37
    // size 64
    chunk[tmp + 4] = malloc(0x35); // 53

    for (int i = 0; i <= tmp; ++i)
        free_chunk(chunk[i]);

    free_chunk(chunk[tmp + 1]);
    free_chunk(chunk[tmp + 2]);
    free_chunk(chunk[tmp + 3]);
    free_chunk(chunk[tmp + 4]);
}