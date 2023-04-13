#include "list.h"
#include "uart1.h"
#include "utils.h"
#include "buddy_system.h"
#include "dtb.h"
#include "exception.h"

#define MEMORY_BASE 0x0      // 0x10000000 - 0x20000000 (SPEC) -> Advanced #3 for all memory region
#define PAGESIZE 0x1000      // 4KB
#define MAX_PAGES 0x10000    // 65536 (Entries), PAGESIZE * MAX_PAGES = 0x10000000 (SPEC)
#define LOG2_MAX_PAGES 16
#define LOG2_MAX_PAGES_PLUS_1 17
#define BELONG_LEFT -1
#define ALLOCATED -2

extern char  _heap_top;
static char* htop_ptr = &_heap_top;

extern char  _start;
extern char  _end;
extern char  _stack_top;
extern char* CPIO_DEFAULT_START;
extern char* CPIO_DEFAULT_END;
extern char* dtb_ptr;

struct buddy_system_node {
    int index;     // index in frame array
    int level;     // 0 ~ 17
    struct list_head list; // free list in this level
};

struct buddy_system_node *node;
struct list_head *head;
int *frame_array;  // level, BELONG_LEFT, ALLOCATED



void* kmalloc(unsigned int size) {
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


void buddy_system_update_list(int index) {
    // checks the level of the newly freed block
    int level = frame_array[index];

    // If the level is at the maximum level or the block is already marked as allocated or it is already marked as belonging to the left half of a larger block,
    if (level == LOG2_MAX_PAGES || level == BELONG_LEFT || level == ALLOCATED)
        return;

    // use the block’s index xor with its exponent to find its buddy (SPEC)
    int buddy = index ^ pow2(level);

    // it has already been allocated or is not of the same size -> do nothing
    if (frame_array[buddy] != level)
        return;

    int frist = index < buddy ? index : buddy;
    int second = index < buddy ? buddy : index;

    // delete the current block and its buddy block from their current free list
    list_del_init(&node[frist].list);
    list_del_init(&node[second].list);
    frame_array[frist] = level + 1;
    frame_array[second] = BELONG_LEFT;
    node[frist].level = level + 1;

    // adds the new, larger block to the free list of the next level
    list_add(&node[frist].list, &head[level + 1]);
    buddy_system_update_list(frist);

    return;
}

void *simple_malloc(void **cur, unsigned long size) {
    void *ret = *cur;
    *cur = *(char **)cur + size;
    return ret;
}

void reserve_memory(unsigned long long start, unsigned long long end) {
    for (int i = 0; i < MAX_PAGES; i++) {
        if (frame_array[i] != 0 && frame_array[i] != ALLOCATED && frame_array[i] != BELONG_LEFT) {
            uart_sendline("i = ");
            uart_2hex(i);
            uart_sendline("\n");
            uart_sendline("frame_array[i] = ");
            uart_2hex(frame_array[i]);
            uart_sendline("\n");
        }
    }
    int start_page = (start - MEMORY_BASE) / PAGESIZE;
    int end_page = (end - MEMORY_BASE - 1) / PAGESIZE;
    // start -= start % PAGESIZE; // floor (align 0x1000)
    // end = end % PAGESIZE ? end + PAGESIZE - (end % PAGESIZE) : end; // ceiling (align 0x1000)

    uart_sendline("Reserved Memory: ");
    uart_sendline("start_page 0x%x ~ ", start_page);
    uart_sendline("end_page 0x%x\r\n",end_page);

    for (int i = start_page; i <= end_page; i++) {
        frame_array[i] = ALLOCATED;
        list_del_init(&node[i].list);
    }
}

void buddy_system_print_all() {
    struct list_head *tmp;
    uart_sendline("----------- list ----------\n");
    for (int i = 0; i < LOG2_MAX_PAGES_PLUS_1; i++) {
        uart_sendline("head[");
        uart_2hex(i);
        uart_sendline("]: ");
        list_for_each(tmp, &head[i]) {
            uart_2hex(list_entry(tmp, struct buddy_system_node, list)->index);
            uart_sendline(" ");
        }
        uart_sendline("\n");
    }
    uart_sendline("\n----------------------------\n");

    return;
}

void buddy_system_init() {
    void *base = (void *)&_end;
    head = (struct list_head *)simple_malloc(&base, (unsigned long)sizeof(struct list_head) * LOG2_MAX_PAGES_PLUS_1);
    node = (struct buddy_system_node *)simple_malloc(&base, (unsigned long)sizeof(struct buddy_system_node) * MAX_PAGES);
    frame_array = (int *)simple_malloc(&base, (unsigned long)sizeof(int) * MAX_PAGES);

    for (int i = 0; i < LOG2_MAX_PAGES_PLUS_1; i++)
        INIT_LIST_HEAD(&head[i]);
    for (int i = 0; i < MAX_PAGES; i++) {
        node[i].index = i;
        INIT_LIST_HEAD(&node[i].list);
        node[i].level = 0;
        frame_array[i] = 0;
        list_add_tail(&node[i].list, &head[0]);
    }

    /* Startup reserving the following region:
    Spin tables for multicore boot (0x0000 - 0x1000)
    Devicetree (Optional, if you have implement it)
    Kernel image in the physical memory
    Your simple allocator (startup allocator) (Stack + Heap in my case)
    Initramfs
    */
    uart_sendline("\r\n* Startup Allocation *\r\n");
    uart_sendline("buddy system: usable memory region: 0x%x ~ 0x%x\n", BUDDY_MEMORY_BASE, BUDDY_MEMORY_BASE + BUDDY_MEMORY_PAGE_COUNT * PAGESIZE);

    uart_sendline("\r\n* dtb_find_and_store *\r\n");
    dtb_find_and_store_reserved_memory(); // find spin tables in dtb

    uart_sendline("\r\n* Reserve kernel: 0x%x ~ 0x%x * \r\n", &_start, &_end);
    reserve_memory((unsigned long long)&_start, (unsigned long long)&_end); // kernel

    uart_sendline("\r\n* Reserve heap & stack: 0x%x ~ 0x%x *\r\n", &_heap_top, &_stack_top);
    reserve_memory((unsigned long long)&_heap_top, (unsigned long long)&_stack_top);  // heap & stack -> simple allocator

    uart_sendline("\r\n* Reserve CPIO: 0x%x ~ 0x%x *\r\n", CPIO_DEFAULT_START, CPIO_DEFAULT_END);
    reserve_memory((unsigned long long)CPIO_DEFAULT_START, (unsigned long long)CPIO_DEFAULT_END);

    for (int i = 0; i < MAX_PAGES; i++)
        buddy_system_update_list(i);

    buddy_system_print_all();
    uart_sendline("\r\n* Finish *\r\n");
    return;
}

int buddy_system_find_suitable_size(int size) {
    int ret = 4;
    while (ret < size)
        ret <<= 1;
    return ret;
}

int buddy_system_split(int level) {
    if (level > LOG2_MAX_PAGES)
        return -1;

    // if the given level is empty, go to next level
    if (list_empty(&head[level]) && buddy_system_split(level + 1) == -1)
        return -1;

    // removes the first block from the list
    struct buddy_system_node *tmp = list_first_entry(&head[level], struct buddy_system_node, list);
    list_del_init(&tmp->list);
    frame_array[tmp->index] = level - 1;
    node[tmp->index].level = level - 1;

    // from index tmp->index ~ tmp->index + pow2(level - 1) -1 is use for "tmp->index"'s Buddy and store BELONG_LEFT
    frame_array[tmp->index + pow2(level - 1)] = level - 1;
    node[tmp->index + pow2(level - 1)].level = level - 1;
    // add to the free list
    list_add(&node[tmp->index + pow2(level - 1)].list, &head[level - 1]);
    list_add(&tmp->list, &head[level - 1]);
    return 1;
}

int buddy_system_find_node_index(int level) {
    if (list_empty(&head[level]) && buddy_system_split(level + 1) == -1)
        return -1;

    return list_first_entry(&head[level], struct buddy_system_node, list)->index;
}

unsigned long int buddy_system_alloc(int size) {
    int suitable_size = buddy_system_find_suitable_size(size);
    int level = log2(suitable_size / 4);  // level0: 4(KB)
    uart_sendline("buddy_system_alloc: ");
    uart_2hex(suitable_size);
    uart_sendline("\nlevel: ");
    uart_2hex(level);
    uart_sendline("\n");
    int index = buddy_system_find_node_index(level);
    if (index == -1) {
        uart_sendline("Can't find suitable node!\n");
        return -1;
    }
    uart_sendline("index: ");
    uart_2hex(index);
    uart_sendline("\n");
    list_del_init(&node[index].list);
    frame_array[index] = ALLOCATED;
    buddy_system_print_all();
    return MEMORY_BASE + index * PAGESIZE;
}

void buddy_system_free(int index) {
    uart_sendline("buddy_system_free index: ");
    uart_2hex(index);
    //uart_send(index);
    uart_sendline("\n");

    if (frame_array[index] != ALLOCATED) {
        uart_sendline("This node is not allocated!\n");
        return;
    }

    int level = node[index].level;
    frame_array[index] = level;
    uart_sendline("level: ");
    uart_2hex(level);
    uart_sendline("\n");

    // node is added to the free list corresponding to its level
    list_add(&node[index].list, &head[level]);
    buddy_system_update_list(index);
    buddy_system_print_all();
}
