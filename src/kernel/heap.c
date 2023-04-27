#include "ds/heap.h"
#include "mem.h"
#include "utils.h"
#include "peripherals/mini_uart.h"

void ds_heap_node_init(struct ds_heap_node* node, void *ptr, uint64_t len, long long pri) {
    node->len = len;
    node->ptr = (void*)simple_malloc(len);
    memcpy(node->ptr, ptr, len);
    node->pri = pri;
}
void ds_heap_init(struct ds_heap *heap) {
    heap->size = 0;
    for(int i = 0; i < DS_HEAP_MAX_LEN; i ++) {
        heap->que[i].pri = 0xffffffffffffffffULL;
    }
}
static void swap_node(struct ds_heap_node* a, struct ds_heap_node* b) {
    struct ds_heap_node temp;

    temp.len = a->len;
    temp.pri = a->pri;
    temp.ptr = a->ptr;
    a->len = b->len;
    a->pri = b->pri;
    a->ptr = b->ptr;
    b->len = temp.len;
    b->pri = temp.pri;
    b->ptr = temp.ptr;

}
static void ds_heap_fix(struct ds_heap *heap) {
    int cur = heap->size - 1;
    while(cur) {
        int pa = ((cur - 1) >> 1);
        if(heap->que[pa].pri > heap->que[cur].pri) {
            swap_node(&(heap->que[pa]), &(heap->que[cur]));
            cur = pa;
        } else {
            break;
        }
    }
}
void ds_heap_push(struct ds_heap* heap, void *ptr, uint64_t len, long long pri) {
    ds_heap_node_init(&(heap->que[heap->size]), ptr, len, pri);
    heap->size += 1;
    ds_heap_fix(heap);
}

void ds_heap_pop(struct ds_heap* heap) {
    swap_node(&(heap->que[0]), &(heap->que[heap->size - 1]));
    heap->que[heap->size - 1].pri = 0xffffffffffffffffLL;
    heap->size -= 1;
    int cur = 0;
    while(cur < heap->size) {
        int right = (cur << 1) + 2;
        int left = (cur << 1) + 1;
        int swap_idx;
        if(heap->que[right].pri < heap->que[left].pri) {
            swap_idx = right;
        } else {
            swap_idx = left;
        }
        if(heap->que[cur].pri > heap->que[swap_idx].pri) {
            swap_node(&(heap->que[cur]), &(heap->que[swap_idx]));
        } else {
            break;
        }
    }
}
void *ds_heap_front(struct ds_heap* heap) {
    if(heap->size == 0) return NULL;
    return heap->que[0].ptr;
}