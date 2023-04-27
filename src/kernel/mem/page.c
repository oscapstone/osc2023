#include "type.h"
#include "mem/page.h"
#include "ds/list.h"
#include "mem/mem.h"
#include "utils.h"
#include "peripherals/mini_uart.h"
#include "dtb/dtb.h"

#define MEM_DEMO

// #define ENTRY_NUM (PAGE_END - PAGE_BEGIN) / PAGE_SIZE

// struct frame_entry entry[ENTRY_NUM];

struct ds_list_head frame_list[PAGE_MAX_ORDER + 1];
struct ds_list_head mem_list;
struct ds_list_head reserve_list;
struct reserve_node {
    uint64_t s;
    uint64_t e;
    struct ds_list_head head;
};


static uint64_t _align_up(uint64_t addr, uint8_t ord) {
    return addr & (~((1LL << ord) - 1LL));
}
static uint64_t _align_down(uint64_t addr, uint8_t ord) {
    if(addr & ((1LL << ord) - 1)) {
        return (addr + (1LL << ord)) & ((~((1LL << ord) - 1LL)));
    }
    return addr;
}
uint64_t order_to_size(uint8_t order) {
    return ((1LL << order) << PAGE_SIZE_ORDER);
}

static uint32_t addr2idx(uint64_t addr, uint64_t base) {
    return (addr - base) >> PAGE_SIZE_ORDER;
}

static uint64_t idx2addr(uint32_t idx, uint64_t base) {
    return ((uint64_t)idx << PAGE_SIZE_ORDER) + base;
}

struct frame_entry* get_entry_from_addr(void *ptr) {
    uint64_t addr = (uint64_t)ptr;
    for(struct ds_list_head *head = mem_list.next; head != &mem_list; head = head->next) {
        struct mem_region *region = container_of(head, struct mem_region, head);
        if(addr >= region->begin_addr && addr <= region->begin_addr + region->size) {
            uint64_t idx = addr2idx(addr, region->begin_addr);
            return &(region->entries[idx]);
        }
    }
}
void init_mem_region(struct mem_region* ptr, uint64_t begin, uint64_t end) {
    ptr->begin_addr = begin;
    ptr->size = end - begin;
    uint8_t ord = 0;
    while(ptr->size >> ord) {
        ord += 1;
        if(ord > PAGE_MAX_ORDER + PAGE_SIZE_ORDER)break;
    }
    ord -= 1;
    ptr->max_ord = ord - PAGE_SIZE_ORDER;
}

void frame_init(uint64_t full_sz) {
    struct mem_region *new_region = simple_malloc(sizeof(struct mem_region));
    init_mem_region(new_region, 0x0, full_sz);
    ds_list_head_init(&(new_region->head));
    ds_list_addnext(&mem_list, &(new_region->head));
    for(struct ds_list_head *tmp = reserve_list.next; tmp != &reserve_list; tmp = tmp->next) {
        struct reserve_node *node = container_of(tmp, struct reserve_node, head);
        for(struct ds_list_head *region = mem_list.next; region != &mem_list; region = region->next) {
            struct mem_region *old_region = container_of(region, struct mem_region, head);
            uint64_t start = old_region->begin_addr;
            uint64_t end = start + old_region->size;
            if(start >= node->e) {
                continue;
            }
            if(end <= node->s) {
                continue;
            }

            //fully contained
            if(start < node->s && end > node->e) {
                if(end - node->e >= PAGE_SIZE) {
                    struct mem_region *new_reg = simple_malloc(sizeof(struct mem_region));
                    init_mem_region(new_reg, node->e, end);
                    ds_list_head_init(&(new_reg->head));
                    ds_list_addnext(region, &(new_reg->head));
                }
                init_mem_region(old_region, start, node->s);
                if(node->s - start < PAGE_SIZE) {
                    ds_list_remove(&(old_region->head));
                }
                break;
            }

            if(start < node->s) {
                init_mem_region(old_region, start, node->s);
                if(node->s - start < PAGE_SIZE) {
                    ds_list_remove(&(old_region->head));
                }
                break;
            }
            if(end > node->e) {
                init_mem_region(old_region, node->e, end);
                if(end - node->e < PAGE_SIZE) {
                    ds_list_remove(&(old_region->head));
                }
                break;
            }
        }
    }
    for(int i = 0; i <= PAGE_MAX_ORDER; i ++) {
        ds_list_head_init(&(frame_list[i]));
    }
    for(struct ds_list_head *head = mem_list.next; head != &mem_list; head = head->next) {
        struct mem_region* region = container_of(head, struct mem_region, head);
#ifdef MEM_DEMO
        uart_send_u64(region->begin_addr);
        uart_send_string(", ");
        uart_send_u64(region->begin_addr + region->size);
        uart_send_string("\r\n");
#endif
        int entry_num = region->size >> PAGE_SIZE_ORDER;
        // uart_send_u64(entry_num * sizeof(struct frame_entry));
        region->entries = (struct frame_entry*)simple_malloc(sizeof(struct frame_entry) * (region->size >> PAGE_SIZE_ORDER));
        for(int i = 0; i < entry_num; i += (1LL << region->max_ord)) {
            region->entries[i].idx = i;
            region->entries[i].mem_region = region;
            // add frame to the list
            ds_list_head_init(&(region->entries[i].head));
            region->entries[i].flag = region->max_ord;
            region->entries[i].order = region->max_ord;
            ds_list_addnext(&frame_list[region->max_ord], &(region->entries[i].head));
        }
    }
}

uint8_t page_size_to_order(uint64_t sz) {

    uint8_t i;
    for(i = 63; i >= 0; i --) {
        if(sz >> i) {
            break;
        }
    }
    if((sz & ((1 << i) - 1)) == 0) {
        return (i >> PAGE_SIZE_ORDER);
    }
    return (i + 1) >> PAGE_SIZE_ORDER;
}

void *_split_page(uint8_t cur_order, uint8_t target_order) {
    // will return a segment with target order
    struct ds_list_head *head = frame_list[cur_order].next;
    ds_list_remove(head);

    struct frame_entry *ent = container_of(head, struct frame_entry, head);
    struct mem_region *region = ent->mem_region;
    // left half

    uint64_t left_addr = idx2addr(ent->idx, region->begin_addr);

#ifdef MEM_DEMO
    uart_send_string("current chunk split addr, size ");
    uart_send_u64(left_addr);
    uart_send_string(", ");
    uart_send_u64(1 << (cur_order + PAGE_SIZE_ORDER));
    uart_send_string("\r\n");
#endif

    if(cur_order == target_order) {
        region->entries[ent->idx].flag = PAGE_ALLOCATED;
        return (void*)left_addr;
    }
    // right half
    uint64_t right_addr = left_addr ^ (1LL << (cur_order - 1 + PAGE_SIZE_ORDER));
    uint32_t right_idx = addr2idx(right_addr, region->begin_addr);

// set entries struct for both left chunk and right chunk
    region->entries[right_idx].flag = cur_order - 1;
    region->entries[right_idx].order = cur_order - 1;
    region->entries[right_idx].idx = right_idx;
    region->entries[right_idx].mem_region = region;
    region->entries[ent->idx].idx = ent->idx;
    region->entries[ent->idx].order = cur_order - 1;
    region->entries[ent->idx].flag = cur_order - 1;

#ifdef MEM_DEMO
    uart_send_string("split new addr, size ");
    uart_send_u64(right_addr);
    uart_send_string(", ");
    uart_send_u64(1LL << (cur_order + PAGE_SIZE_ORDER - 1));
    uart_send_string("\r\n");
#endif
// push entries into list
    ds_list_head_init(&(region->entries[right_idx].head));
    ds_list_head_init(&(region->entries[ent->idx].head));
    ds_list_addnext(&frame_list[cur_order - 1], &(region->entries[ent->idx].head));
    ds_list_addnext(&frame_list[cur_order - 1], &(region->entries[right_idx].head));
    return _split_page(cur_order - 1, target_order);
}

void *_find_page(uint8_t order) {

    struct ds_list_head *front = ds_list_front(&(frame_list[order]));
    if(front != NULL) {
        struct ds_list_head *head = frame_list[order].next;
        struct frame_entry *ent = container_of(head, struct frame_entry, head);
        ent->flag = PAGE_ALLOCATED;
        ds_list_remove(frame_list[order].next);
        uart_send_string("Successfully find page\r\n");
        return (void*)idx2addr(ent->idx, ent->mem_region->begin_addr);
    }
    
    // if not found small page, look for larger page

    for(uint8_t i = order; i <= PAGE_MAX_ORDER; i ++) {
        struct ds_list_head *front = ds_list_front(&(frame_list[i]));
        // if order i has page
        // split it down to target order
        if(front != NULL) {
            void *ptr = _split_page(i, order);
            return ptr;
        }
    }
    // if still not found
    return NULL;
}

void *page_alloc(uint64_t size) {
    int order = page_size_to_order(size);
    uart_send_dec(order);
    uart_send_string("\r\n");
    if(order > PAGE_MAX_ORDER) {
        return NULL;
    }
    void *ptr = _find_page(order);
    uart_send_string("Allocated page: ");
    uart_send_u64((uint64_t)ptr);
    uart_send_string("\r\n");
    return ptr;
}
void _merge_page(void *addr, uint8_t cur_order, struct mem_region *region) {
    // if(cur_order >= PAGE_MAX_ORDER) {
    //     return;
    // }

    uint32_t idx = addr2idx((uint64_t)addr, region->begin_addr);
    uint64_t ano_addr = (uint64_t)addr ^ (1LL << (cur_order + PAGE_SIZE_ORDER));
    uint32_t ano_idx = addr2idx((uint64_t)ano_addr, region->begin_addr);
    if(cur_order < region->max_ord && region->entries[ano_idx].flag == cur_order) {
        #ifdef MEM_DEMO
        uart_send_string("Merge Page addr, addr, size ");
        uart_send_u64((uint64_t)addr);
        uart_send_string(", ");
        uart_send_u64(ano_addr);
        uart_send_string(", ");
        uart_send_u64((1LL << (cur_order + PAGE_SIZE_ORDER)));
        uart_send_string("\r\n");
        #endif
        ds_list_remove(&(region->entries[ano_idx].head));
        // happen when addr is left
        if(ano_addr > (uint64_t)addr) {
            region->entries[ano_idx].flag = PAGE_CONTAINED;
            _merge_page((void*)addr, cur_order + 1, region);
        } else {
            region->entries[idx].flag = PAGE_CONTAINED;
            _merge_page((void*)ano_addr, cur_order + 1, region);
        }
    } else {
        region->entries[idx].flag = cur_order;
        region->entries[idx].order = cur_order;
        ds_list_head_init(&(region->entries[idx].head));
        ds_list_addnext(&frame_list[cur_order], &(region->entries[idx].head));
    }
}

void page_free(void *addr) {
    struct mem_region *region;
    for(struct ds_list_head *head = mem_list.next; head != &(mem_list); head = head->next) {
        region = container_of(head, struct mem_region, head);
        if(region->begin_addr <= (uint64_t)addr && region->begin_addr + region->size >= (uint64_t)addr) {
            break;
        }
    }
    uint32_t idx = addr2idx((uint64_t)addr, region->begin_addr);
    _merge_page(addr, region->entries[idx].order, region);
}
void memory_reserve(uint64_t start, uint64_t end) {
    struct reserve_node* node = simple_malloc(sizeof(struct reserve_node));
    ds_list_head_init(&(node->head));
    start = _align_down(start, PAGE_SIZE_ORDER);
    end = _align_up(end, PAGE_SIZE_ORDER);
    node->s = start;
    node->e = end;
#ifdef MEM_DEMO
    uart_send_string("Memory Reserve\r\n");
    uart_send_u64(start);
    uart_send_string(", ");
    uart_send_u64(end);
    uart_send_string("\r\n");
#endif
    ds_list_addnext(&(reserve_list), &(node->head));
}