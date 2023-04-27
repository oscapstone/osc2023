/*
This is a simple queue
Not used in lab3
*/
#include "ds/queue.h"
#include "mem/mem.h"
#include "utils.h"
#include "peripherals/mini_uart.h"
void ds_queue_node_init(struct ds_queue_node* node, void *ptr, uint64_t len) {
    node->len = len;
    node->ptr = (void*)simple_malloc(len);
    memcpy(node->ptr, ptr, len);
    node->nxt = NULL;
}
void ds_queue_init(struct ds_queue *que) {
    que->front = NULL;
    que->back = NULL;
    que->size = 0;
}
void ds_queue_push(struct ds_queue *que, void *ptr, uint64_t len) {
    struct ds_queue_node* new_node = (struct ds_queue_node*)simple_malloc(sizeof(struct ds_queue_node));
    ds_queue_node_init(new_node, ptr, len);
    que->size += 1;
    if(que->size == 0) {
        que->back = que->front = new_node;
    } else {
        que->back->nxt = new_node;
        que->back = new_node;
    }
}
void ds_queue_pop(struct ds_queue *que) {
    if(que->size == 0) {
        uart_send_string("[Error] queue pop\r\n");
        return;
    }
    que->size -= 1;
    que->front = que->front->nxt;
}
void *ds_queue_front(struct ds_queue *que) {
    if(que->size == 0) {
        return NULL;
    }
    return que->front->ptr;
}