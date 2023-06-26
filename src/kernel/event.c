#include "event.h"
#include "ds/heap.h"
#include "interrupt.h"
#include "utils.h"
#include "peripherals/mini_uart.h"

/* this is cursed code :(*/
struct k_event_queue event_queue = {0};

void k_event_queue_init() {
    // init k_event_queue heap
    ds_heap_init(&(event_queue.heap));
    event_queue.cur_max_event_pri = -0xffffffffffffffff;
}

void k_event_init(struct k_event* event, void (*cb)(void *, uint32_t)) {
    event->ptr_len = 0;
    event->cb = cb;
}

void k_event_submit(struct k_event* event, void *ptr, uint32_t ptr_len, long long pri) {
    // submit event;
    if(ptr != NULL) {
        event->ptr = (void *)simple_malloc(ptr_len);
        event->ptr_len = ptr_len;
        memcpy(event->ptr, ptr, ptr_len);
    } else {
        ptr = NULL;
    }
    k_event_queue_push(&event_queue, event, pri);
}

void k_event_queue_push(struct k_event_queue* que, struct k_event *event, long long pri) {
    int flag = interrupt_disable_save();
    // don't need to consider the data inside ptr, since in ds_heap_push it only copy pointer value.
    // which means that it only 8 bytes!!!
    event->preempt = 0;
    event->pri = pri;
    ds_heap_push(&(que->heap), event, sizeof(struct k_event), -pri);
    interrupt_enable_restore(flag);
}

struct k_event* k_event_front() {
    int flag = interrupt_disable_save();
    struct k_event *ke;
    if(event_queue.heap.size == 0) {
        ke = NULL;
        goto ret;
    }
    ke = ds_heap_front(&(event_queue.heap));
ret:
    interrupt_enable_restore(flag);
    return ke;
}

void k_event_queue_pop() {
    int flag = interrupt_disable_save();
    ds_heap_pop(&(event_queue.heap));
    interrupt_enable_restore(flag);
}

void switch_preept_status(struct k_event* event, uint16_t stat) {
    int flag = interrupt_disable_save();
    event->preempt = stat;
    interrupt_enable_restore(flag);
}

void event_queue_handle() {
    // this weird handler should work properly
    struct k_event *event;
    int counter = 0;
    while(event_queue.heap.size) {
        event = k_event_front();
        if(event == NULL) {
            return;
        }
        if(event->preempt == 3) return;

        event->preempt = 3;
        event->cb(event->ptr, event->ptr_len);
        irqhandler_dec();
        k_event_queue_pop();
        if(irqhandler_cnt_get() == 0) {
            event_queue.cur_max_event_pri = -0x7fffffffffffffff;
        }
        counter += 1;

    }
}