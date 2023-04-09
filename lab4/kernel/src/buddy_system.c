#include "list.h"
#include "uart1.h"
#include "utils.h"
#include "buddy_system.h"
#include "dtb.h"
#include "exception.h"

#define MEMORY_BASE 0x0
#define PAGESIZE 0x1000 // 4KB
#define MAX_PAGES 0x40000
#define LOG2_MAX_PAGES 18
#define LOG2_MAX_PAGES_PLUS_1 19
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
    int index;
    int level;
    struct list_head list;
};

struct buddy_system_node *node;
struct list_head *head;
int *frame_array;


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


void buddy_system_update_list(int index) {
    int level = frame_array[index];
    if (level == LOG2_MAX_PAGES || level == BELONG_LEFT || level == ALLOCATED)
        return;

    int buddy = index ^ pow2(level);
    if (frame_array[buddy] != level)
        return;

    int frist = index < buddy ? index : buddy;
    int second = index < buddy ? buddy : index;

    list_del_init(&node[frist].list);
    list_del_init(&node[second].list);
    frame_array[frist] = level + 1;
    frame_array[second] = BELONG_LEFT;
    node[frist].level = level + 1;
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
            uart_puts("i = ");
            uart_2hex(i);
            uart_puts("\n");
            uart_puts("frame_array[i] = ");
            uart_2hex(frame_array[i]);
            uart_puts("\n");
        }
    }
    int start_page = (start - MEMORY_BASE) / PAGESIZE;
    int end_page = (end - MEMORY_BASE - 1) / PAGESIZE;
    uart_puts("start_page: ");
    uart_2hex(start_page);
    uart_puts("\n");
    uart_puts("end_page: ");
    uart_2hex(end_page);
    uart_puts("\n");
    for (int i = start_page; i <= end_page; i++) {
        frame_array[i] = ALLOCATED;
        list_del_init(&node[i].list);
    }
}

void buddy_system_print_all() {
    struct list_head *tmp;
    uart_puts("----------- list ----------\n");
    for (int i = 0; i < LOG2_MAX_PAGES_PLUS_1; i++) {
        uart_puts("head[");
        uart_2hex(i);
        uart_puts("]: ");
        list_for_each(tmp, &head[i]) {
            uart_2hex(list_entry(tmp, struct buddy_system_node, list)->index);
            uart_puts(" ");
        }
        uart_puts("\n");
    }
    // uart_puts("----------- array ----------");
    // for (int i = 0; i < MAX_PAGES; i++) {
    //     if (i % 8 == 0)
    //         uart_puts("\n");
    //     if (frame_array[i] == BELONG_LEFT)
    //         uart_puts("--------");
    //     else if (frame_array[i] == ALLOCATED)
    //         uart_puts("XXXXXXXX");
    //     else
    //         uart_2hex(frame_array[i]);
    //     uart_puts(" ");
    // }
    uart_puts("\n----------------------------\n");

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
    dtb_find_and_store_reserved_memory(); // find spin tables in dtb
    reserve_memory((unsigned long long)&_start, (unsigned long long)&_end); // kernel
    reserve_memory((unsigned long long)&_heap_top, (unsigned long long)&_stack_top);  // heap & stack -> simple allocator
    reserve_memory((unsigned long long)CPIO_DEFAULT_START, (unsigned long long)CPIO_DEFAULT_END);

    for (int i = 0; i < MAX_PAGES; i++)
        buddy_system_update_list(i);

    buddy_system_print_all();
    return;
}

int buddy_system_find_suitable_size(int size) {
    int ret = 4;
    while (ret < size)
        ret <<= 1;
    return ret;
}

int buddy_system_spilt(int level) {
    if (level > LOG2_MAX_PAGES)
        return -1;

    if (list_empty(&head[level]) && buddy_system_spilt(level + 1) == -1)
        return -1;

    struct buddy_system_node *tmp = list_first_entry(&head[level], struct buddy_system_node, list);
    list_del_init(&tmp->list);
    frame_array[tmp->index] = level - 1;
    node[tmp->index].level = level - 1;
    frame_array[tmp->index + pow2(level - 1)] = level - 1;
    node[tmp->index + pow2(level - 1)].level = level - 1;
    list_add(&node[tmp->index + pow2(level - 1)].list, &head[level - 1]);
    list_add(&tmp->list, &head[level - 1]);
    return 1;
}

int buddy_system_find_node_index(int level) {
    if (list_empty(&head[level]) && buddy_system_spilt(level + 1) == -1)
        return -1;

    return list_first_entry(&head[level], struct buddy_system_node, list)->index;
}

unsigned long int buddy_system_alloc(int size) {
    int suitable_size = buddy_system_find_suitable_size(size);
    int level = log2(suitable_size / 4);
    uart_puts("buddy_system_alloc: ");
    uart_2hex(suitable_size);
    uart_puts("\nlevel: ");
    uart_2hex(level);
    uart_puts("\n");
    int index = buddy_system_find_node_index(level);
    if (index == -1) {
        uart_puts("Can't find suitable node!\n");
        return -1;
    }
    uart_puts("index: ");
    uart_2hex(index);
    uart_puts("\n");
    list_del_init(&node[index].list);
    frame_array[index] = ALLOCATED;
    buddy_system_print_all();
    return MEMORY_BASE + index * PAGESIZE;
}

void buddy_system_free(int index) {
    uart_puts("buddy_system_free index: ");
    uart_2hex(index);
    uart_puts("\n");

    if (frame_array[index] != ALLOCATED) {
        uart_puts("This node is not allocated!\n");
        return;
    }

    int level = node[index].level;
    frame_array[index] = level;
    uart_puts("level: ");
    uart_2hex(level);
    uart_puts("\n");

    list_add(&node[index].list, &head[level]);
    buddy_system_update_list(index);
    buddy_system_print_all();
}
