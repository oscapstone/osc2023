#ifndef __HEAP_H
#define __HEAP_H
#include "type.h"
#include "mem.h"

#define DS_HEAP_MAX_LEN 256
struct ds_heap_node {
    long long pri;
    uint64_t len;
    void *ptr;
};

struct ds_heap {
    struct ds_heap_node que[DS_HEAP_MAX_LEN];
    uint64_t size;
};
void ds_heap_node_init(struct ds_heap_node* heap, void *ptr, uint64_t len,long long pri);
void ds_heap_init(struct ds_heap* heap);
void ds_heap_push(struct ds_heap* heap, void *ptr, uint64_t len, long long pri);
void *ds_heap_front(struct ds_heap* heap);
void ds_heap_pop(struct ds_heap* heap);
#endif