#include "ds/ringbuffer.h"
#include "mem/mem.h"
/*
This is a simple queue
Not used in lab3
*/
void* ringbuffer_alloc(uint32_t sz) {
    struct ringbuffer* ret = (struct ringbuffer*)simple_malloc(sizeof(struct ringbuffer));
    ret->ptr = (void*)simple_malloc(sz);
    ret->begin = 0;
    ret->end = 0;
    ret->size = 0;
    ret->max_len = sz;
    return ret;
}

void ringbuffer_push(struct ringbuffer* buf, void *ptr, uint32_t data_len) {
    for(int i = 0, j = buf->end; i < data_len; i++, j ++) {
        *(char*)(buf->ptr + j) = *(char*)(ptr + i);
        if(j == buf->max_len) {
            j = 0;
        }
    }
}

void ringbuffer_pop(struct ringbuffer *buf) {
    buf->begin += 1;
    buf->size -= 1;
}
char ringbuffer_get(struct ringbuffer *buf, uint32_t idx){
    return *(char*)(buf->ptr + idx);
}

void ringbuffer_clear(struct ringbuffer *buf) {
    kfree(buf->ptr);
    buf = ringbuffer_alloc(buf->max_len);
}