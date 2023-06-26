#include "mem/mem.h"
#include "peripherals/mini_uart.h"
#include "mem/page.h"
#include "ds/list.h"
#include "utils.h"
#include "interrupt.h"

#define SMALL_CHUNK_MAX_ORDER (PAGE_SIZE_ORDER)
#define SMEM_CHUNK_MAX 31
#define ALLOC_MIN_ORD 5
#define MIN_FREE_PAGE_CNT 20

// #define MEM_DEMO

extern uint32_t _heap_begin;
void* cur_mem_offset = &_heap_begin;

extern struct ds_list_head mem_list;
extern struct ds_list_head reserve_list;
struct ds_list_head kmem_pool[SMALL_CHUNK_MAX_ORDER + 1];
struct ds_list_head smem_pool[SMEM_CHUNK_MAX + 1];

struct smem_node {
    uint64_t size;
    struct ds_list_head head;
};
struct kmem_node {
    uint64_t addr;
    struct ds_list_head head;
};

uint8_t size2ord(uint32_t sz) {
    uint8_t i;
    for(i = 31; i >= 0; i --) {
        if(sz >> i) {
            break;
        }
    }
    if((sz & ((1U << i) - 1)) == 0) {
        return i;
    }
    return i + 1;
}

void smem_init() {
    ds_list_head_init(&reserve_list);
    ds_list_head_init(&mem_list);
    for(int i = 0; i <= SMEM_CHUNK_MAX; i ++) {
        ds_list_head_init(&(smem_pool[i]));
    }
}

void *simple_malloc(uint32_t size) {
    // pad to align 8 bytes

    uint32_t flag = interrupt_disable_save();
    uint8_t ord = size2ord(size);
    if(ord < 5) {
        ord = 5;
    }

    void *ret;
    struct ds_list_head *front = ds_list_front(&(smem_pool[ord]));
    if(front == NULL) {
        void *ptr = (void *)cur_mem_offset;
        struct smem_node *node = (struct smem_node*)ptr;
        node->size = (1LL << ord);
        cur_mem_offset += ((1LL << ord) + 32);
        ret = (void*)((uint64_t)ptr + 32);
    } else {
        struct smem_node *node = container_of(front, struct smem_node, head);
        ds_list_remove(front);
        void *ptr = ((uint64_t)node + 32);
        ret = ptr;
    }
    interrupt_enable_restore(flag);
    return ret;
}

void simple_free(void *ptr) {
    uint32_t flag = interrupt_disable_save();
    struct smem_node *node = (struct smem_node*)((uint64_t)ptr - 32);
    uint8_t ord = size2ord(node->size);
    ds_list_head_init(&(node->head));
    ds_list_addnext(&(smem_pool[ord]), &(node->head));
    interrupt_enable_restore(flag);
}


void kmalloc_init() {
    for(int i = 0; i < SMALL_CHUNK_MAX_ORDER + 1; i ++) {
        ds_list_head_init(&(kmem_pool[i]));
    }
}


void init_page_chunk(void *ptr, struct frame_entry *ent, uint8_t ord) {
    ds_list_head_init(&(ent->chunk_head));
    for(int i = 0; i < PAGE_SIZE; i += (1 << ord)) {
        struct kmem_node *new_node = simple_malloc(sizeof(struct kmem_node));
        ds_list_head_init(&(new_node->head));
        new_node->addr = (uint64_t)(ptr + i);
        // ds_list_addnext(&kmem_pool[ord], &(new_node->head));
        ds_list_addnext(&(ent->chunk_head), &(new_node->head));
    }
    ds_list_head_init(&(ent->head));
    ds_list_addnext(&(kmem_pool[ord]), &(ent->head));
}

void *kmalloc(uint32_t size) {
    uint32_t flag = interrupt_disable_save();
    if(size == 0){
        goto _r;
    }

    void *ret;
    if(size >= (PAGE_SIZE >> 1)) {
        ret = page_alloc(size);
        goto _r;
    }
    uint8_t ord = size2ord(size);
    if(ord < ALLOC_MIN_ORD) {
        ord = ALLOC_MIN_ORD;
    }
    struct ds_list_head *front = ds_list_front(&kmem_pool[ord]);
    if(front != NULL) {
        // allocated from pre allocated page
        struct frame_entry *ent = container_of(front, struct frame_entry, head);
        front = ds_list_front(&(ent->chunk_head));
        struct kmem_node *node = container_of(front, struct kmem_node, head);
        ret = node->addr;

        // remove from page pool
        ds_list_remove(&(node->head));
        ent->dyn_count -= 1;

        if(ent->dyn_count == 0) {
            // temporal remove frame entry from pre allocated pool
            // since no more chunk can be allocate
            ds_list_remove(&(ent->head));
        }

        // remember the entry was allocate by simple malloc
        simple_free(node);
        goto _r;
    }
    else {
        // need to alloc new page
        void *ptr = page_alloc(PAGE_SIZE);
        struct frame_entry *ent = get_entry_from_addr(ptr);
        ent->flag = PAGE_ALLOCATED_BY_DYNAMIC;
        ent->dyn_ord = ord;
        ent->dyn_count = (1 << (PAGE_SIZE_ORDER - ord));
        init_page_chunk(ptr, ent, ord);
#ifdef MEM_DEMO
        uart_send_string("\r\n");
        uart_send_string("Small Chunk new Base\r\n");
        uart_send_u64(ptr);
        uart_send_string("\r\n");
        uart_send_u64(ent);
        uart_send_string("\r\n");
#endif
        struct ds_list_head *front = ds_list_front(&(ent->chunk_head));
        struct kmem_node *node = container_of(front, struct kmem_node, head);
        ret = node->addr;
        ent->dyn_count -= 1;
        ds_list_remove(front);
        simple_free(node);
    }
_r:
    interrupt_enable_restore(flag);
    #ifdef MEM_DEMO
        uart_send_string("size = ");
        uart_send_dec(size);
        uart_send_string("\r\n");
        uart_send_string("ret = ");
        uart_send_u64(ret);
        uart_send_string("\r\n");
    #endif
    return ret;
}

void free_chunk(struct frame_entry *ent) {
    uint8_t ord = ent->dyn_ord;
    struct ds_list_head *front = ds_list_front(&(ent->chunk_head));
    while(front != NULL) {
        struct kmem_node *node = container_of(front, struct kmem_node, head);
        ds_list_remove(&(node->head));
        simple_free(node);
        front = ds_list_front(&(ent->chunk_head));
    }
}

void kfree(void *ptr) {
    #ifdef MEM_DEMO
        uart_send_string("kfree ptr = ");
        uart_send_u64(ptr);
        uart_send_string("\r\n");
    #endif
    if(ptr == NULL) return;
    uint32_t flag = interrupt_disable_save();
    struct frame_entry *ent = get_entry_from_addr(ptr);
    // We does not handle the address freed is not valid
    if(ent->flag == PAGE_ALLOCATED_BY_DYNAMIC) {
        uint8_t ord = ent->dyn_ord;
        if(ent->dyn_count == 0) {
            ds_list_head_init(&(ent->head));
            ds_list_addnext(&kmem_pool[ent->dyn_ord], &(ent->head));
        }
        ent->dyn_count += 1;
        if(ent->dyn_count == (1 << (PAGE_SIZE_ORDER - ord))) {
            if(ent->used_cnt < MIN_FREE_PAGE_CNT) {
                ent->used_cnt += 1;
            } else {
                ds_list_remove(&(ent->head));
                free_chunk(ent);
                page_free((uint64_t)ptr & (~((1LL << PAGE_SIZE_ORDER) - 1)));
                goto _r;
            }
        }
        // need to add back the frame entry to kmem_pool
        struct kmem_node *node = simple_malloc(sizeof(struct kmem_node));
        node->addr = (uint64_t)ptr;
        ds_list_head_init(&(node->head));
        ds_list_addnext(&(ent->chunk_head), &(node->head));
    } else {
        page_free(ptr);
    }
_r:
    interrupt_enable_restore(flag);
    return;
}
void set_init_mem_region(char *name, char *prop_name, char *data) {

    if(strncmp(name, "memory@0", 8) == 0) {
        if(strncmp(prop_name, "reg", 3) == 0) {
            unsigned long long a = *(unsigned int*)data;
            unsigned long long b = *(unsigned int*)(data + 4);
            b = ((b & 0xff) << 24) + ((b & (0xff << 8)) << 8) + ((b & (0xff << 16)) >> 8) + ((b & (0xff << 24)) >> 24);
            a = ((a & 0xff) << 24) + ((a & (0xff << 8)) << 8) + ((a & (0xff << 16)) >> 8) + ((a & (0xff << 24)) >> 24);
            unsigned long long sz = a + b;
            frame_init(sz);
        }
    }
}
void *cmalloc(uint32_t size) {
    void *ret = kmalloc(size);
    for(uint32_t k = 0; k < (size >> 3); k ++) {
        *((uint64_t*)(ret) + k) = 0;
    }
    uint32_t tmp = ((size >> 3) << 3);
    for(int i = tmp; i < size; i ++) {
        *(char *)(ret + i) = 0;
    }
    return ret;
}