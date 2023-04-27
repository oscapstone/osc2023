#ifndef __TIME_H
#define __TIME_H

#include "type.h"
#include "ds/heap.h"
#define K_TIMEOUT_QUEUE_SIZE 256
#define K_TIMEOUT_ARG_MAX_LEN 32
extern void core_timer_handler();
// void set_timeout(unsigned int sec);

struct k_timeout {
    uint64_t endtick;
    void (*cb)(void*);
    uint32_t que_id;
    void *arg;
};
struct k_timeout_queue {
    struct ds_heap heap;
};

void k_timeout_init(struct k_timeout *time, void (*cb)(void*));
void k_timeout_submit(struct k_timeout *time, char *arg, uint64_t sec);
void k_timeout_queue_push(struct k_timeout_queue *que, struct k_timeout *time);
void k_timeout_queue_pop();
struct k_timeout* k_timeout_queue_front();
void check_k_timeout_queue();
#endif