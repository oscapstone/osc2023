#include "waitqueue.h"
#include "thread.h"
#include "ds/list.h"
#include "type.h"
#include "utils.h"
#include "peripherals/mini_uart.h"

extern struct ds_list_head run_queue;

void waitqueue_init(struct waitqueue_t *queue) {
    ds_list_head_init(&(queue->th_list));
}

void wait(struct waitqueue_t *queue) {
    uint64_t flag = interrupt_disable_save();
    struct Thread_t* th = thread_get_current_instance();

    signal_check();
    th->status = TH_WAIT;
    th->waitqueue = queue;
    schedule(0);
    interrupt_enable_restore(flag);
    return;
}

void waitthread(struct Thread_t *th) {
    uint64_t flag = interrupt_disable_save();

    th->status = TH_WAIT;
    th->waitqueue = NULL;
    schedule(0);
    interrupt_enable_restore(flag);
    return;
}

void wakeupthread(struct Thread_t *th) {
    th->status = TH_IN_RUNNING;
    ds_list_head_init(&(th->tq_head));
    ds_list_addprev(&(run_queue), &(th->tq_head));
    return;
}

void wakeup(struct waitqueue_t *queue) {
    struct ds_list_head *front = ds_list_front(&(queue->th_list));
    if(front == NULL) {
        return;
    }
    struct Thread_t *th = container_of(front, struct Thread_t, tq_head);

    ds_list_remove(&(th->tq_head));
    th->status = TH_IN_RUNNING;
    th->waitqueue = NULL;
    ds_list_head_init(&(th->tq_head));
    ds_list_addprev(&(run_queue), &(th->tq_head));
    return;
}