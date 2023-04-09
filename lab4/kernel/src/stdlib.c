#include "buddy_system.h"
#include "list.h"
#include "uart1.h"
#include "stddef.h"
#include "list.h"

#define BLOCK_SIZE 32

struct block {
    size_t size;
    int free;
    int page_number;
    struct list_head list;
};

struct list_head block_head;
int page_number = 0;

void mem_print_all() {
    struct list_head *tmp;

    uart_puts("----------- block ----------\n");
    list_for_each(tmp, &block_head) {
        struct block *block = list_entry(tmp, struct block, list);
        uart_puts("address: ");
        uart_2hex((unsigned long int)block);
        uart_puts(" size: ");
        uart_2hex(block->size);
        uart_puts(" free: ");
        uart_2hex(block->free);
        uart_puts("\n");
    }
    uart_puts("----------------------------\n");
}

void mem_init() {
    INIT_LIST_HEAD(&block_head);
    struct block *block = (struct block *)buddy_system_alloc(4);
    block->size = 4096 - BLOCK_SIZE;
    block->free = 1;
    block->page_number = page_number++;
    list_add(&block->list, &block_head);
    mem_print_all();
    return;
}

void *malloc(size_t size) {
    int len = size;
    struct list_head *tmp;
    while (len % BLOCK_SIZE)
        len++;

    list_for_each(tmp, &block_head) {
        struct block *block = list_entry(tmp, struct block, list);
        if (block->free && block->size == len) {
            block->free = 0;
            mem_print_all();
            return (void *)block + BLOCK_SIZE;
        }
        if (block->free && block->size >= len) {
            len += BLOCK_SIZE;
            struct block *ptr = block + len / BLOCK_SIZE;
            ptr->size = block->size - len;
            ptr->free = 1;
            ptr->page_number = block->page_number;
            list_add(&ptr->list, &block->list);
            block->size = len - BLOCK_SIZE;
            block->free = 0;
            mem_print_all();
            return (void *)block + BLOCK_SIZE;
        }
    }

    int request = (size + BLOCK_SIZE) / 4096;
    if ((size + BLOCK_SIZE) % 4096 != 0)
        request++;
    request *= 4;
    struct block *block = (struct block *)buddy_system_alloc(request);
    block->size = request / 4 * 4096 - BLOCK_SIZE;
    block->free = 1;
    block->page_number = page_number++;
    list_add_tail(&block->list, &block_head);

    return malloc(size);
}

void merge(struct block *f, struct block *s) {
    f->size += s->size + BLOCK_SIZE;
    list_del(&s->list);
    return;
}

void free_page(unsigned long int ptr) {
    struct block *block = (struct block *)(ptr - BLOCK_SIZE);
    block->free = 1;
    if (block->list.next != &block_head && list_entry(block->list.next, struct block, list)->free)
        if (block->page_number == list_entry(block->list.next, struct block, list)->page_number)
            merge(block, list_entry(block->list.next, struct block, list));
    if (block->list.prev != &block_head && list_entry(block->list.prev, struct block, list)->free)
        if (block->page_number == list_entry(block->list.prev, struct block, list)->page_number)
            merge(list_entry(block->list.prev, struct block, list), block);
    mem_print_all();
}

void free(void* ptr) {
    // TBD
}