#ifndef __DS_QUEUE_H
#define __DS_QUEUE_H

#include "type.h"
struct ds_queue_node {
    uint32_t len;
    struct ds_queue_node *nxt;
    void *ptr;
};
struct ds_queue {
    struct ds_queue_node *front;
    struct ds_queue_node *back;
    uint32_t size;
};
void ds_queue_node_init(struct ds_queue_node* node, void *ptr, uint64_t len);
void ds_queue_init(struct ds_queue *que);
void ds_queue_push(struct ds_queue *que, void *ptr, uint64_t len);
void ds_queue_pop(struct ds_queue *que);
void *ds_queue_front(struct ds_queue *que) ;
#endif
