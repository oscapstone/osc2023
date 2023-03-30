#ifndef __EVENT_H
#define __EVENT_H
#include "type.h"
#include "ds/heap.h"

struct k_event {
    uint32_t ptr_len;
    uint16_t preempt;
    long long pri;
    void (*cb)(void*, uint32_t);
    void *ptr;
};
struct k_event_queue {
    struct ds_heap heap;
    long long cur_max_event_pri;
};
void k_event_queue_init();
void k_event_init(struct k_event* event, void (*cb)(void *, uint32_t));
void k_event_submit(struct k_event* event, void *ptr, uint32_t ptr_len, uint64_t pri);
void k_event_queue_push(struct k_event_queue* que, struct k_event *event, uint64_t pri);
void k_event_queue_pop();
struct k_event* k_event_queue_front();
#endif