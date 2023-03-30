#ifndef __RINGBUFFER_H
#define __RINGBUFFER_H
#include "type.h"

struct ringbuffer {
    uint32_t max_len;
    uint32_t size;
    uint32_t begin;
    uint32_t end;
    void *ptr;
};
void* ringbuffer_alloc(uint32_t sz);
void ringbuffer_push(struct ringbuffer *buf, void *data_ptr, uint32_t data_sz);
void ringbuffer_pop(struct ringbuffer *buf);
char ringbuffer_get(struct ringbuffer *buf, uint32_t idx);
void ringbuffer_clear(struct ringbuffer *buf);
#endif