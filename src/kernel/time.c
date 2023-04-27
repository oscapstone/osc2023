/*
time.c provide timeout queue
that can register function
callabck with certain timeout
*/


#include "time.h"
#include "peripherals/mini_uart.h"
#include "type.h"
#include "mem/mem.h"
#include "ds/heap.h"
#include "interrupt.h"
#include "utils.h"

#define EXPIRE_PERIOD 0xfffffff

static struct k_timeout_queue time_queue;

void k_timeout_init(struct k_timeout *time, void (*cb)(void*)) {
    time->cb = cb;
    time->endtick = 0;
    time->que_id = 0;
    ds_heap_init(&(time_queue.heap));
}
void k_timeout_submit(struct k_timeout *time, char *arg, uint64_t sec) {
    // submit timeout
    uint64_t freq, clock;
    asm volatile("mrs %0, cntpct_el0\n":"=r"(clock));
    asm volatile("mrs %0, cntfrq_el0\n":"=r"(freq));
    if(arg != NULL) {
        // copy arg to new memory buffer
        time->arg = (void*)simple_malloc(K_TIMEOUT_ARG_MAX_LEN);
        strncpy(time->arg, arg, K_TIMEOUT_ARG_MAX_LEN);
    } else {
        time->arg = NULL;
    }
    // setup endtick
    time->endtick = clock + sec * freq;

    k_timeout_queue_push(&time_queue, time);
}
void k_timeout_queue_push(struct k_timeout_queue *que, struct k_timeout *time) {
    // push timeout object into heap
    // remainder: heap will copy every thing in the struct
    int flag = interrupt_disable_save();
    // disable_interrupt();
    ds_heap_push(&(que->heap), time, sizeof(struct k_timeout), time->endtick);
    struct k_timeout *f = ds_heap_front(&(que->heap));
    uint64_t x = f->endtick;
    //set cntp_cval_el0 with certain tick number
    asm volatile(
        "msr cntp_cval_el0, %0\n":"=r"(x)
    );
    // enable_interrupt();
    interrupt_enable_restore(flag);
}
void k_timeout_queue_pop() {
    // atomic pop for time_queue
    int flag = interrupt_disable_save();
    // disable_interrupt();
    ds_heap_pop(&(time_queue.heap));
    // enable_interrupt();
    interrupt_enable_restore(flag);
}
struct k_timeout* k_timeout_queue_front() {
    // disable_interrupt();
    int flag = interrupt_disable_save();
    struct k_timeout *f;
    if(time_queue.heap.size == 0) {
        f = NULL;
        goto ret;
    } 
    f = ds_heap_front(&(time_queue.heap));
ret:
    interrupt_enable_restore(flag);
    // enable_interrupt();
    return f;
}