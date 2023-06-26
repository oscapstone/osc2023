#include <ringbuffer.h>

#include <allocator.h>
#include <interrupt.h>

RingBuffer *RingBuffer_new(size_t bufsize){
    RingBuffer* newbuf = (RingBuffer*)simple_malloc(sizeof(RingBuffer));
    newbuf->flags = 0;
    newbuf->flags |= RingBuffer_flag_Empty;
    newbuf->buf = (char *)simple_malloc(bufsize);
    newbuf->lbound = 0;
    newbuf->rbound = 0;
    newbuf->len = 0;
    newbuf->size = bufsize;
    return newbuf;
}

size_t RingBuffer_writeb(RingBuffer *rbuf, char b){
    interrupt_disable();
    if(RingBuffer_Full(rbuf)) return 0;
    rbuf->buf[rbuf->rbound++] = b;
    rbuf->rbound %= rbuf->size;
    rbuf->len++;
    if(rbuf->len >= rbuf->size) rbuf->flags |= RingBuffer_flag_Full;
    rbuf->flags &= ~RingBuffer_flag_Empty;
    interrupt_enable();
    return 1;
}

size_t RingBuffer_readb(RingBuffer *rbuf, char* b){
    interrupt_disable();
    if(RingBuffer_Empty(rbuf)) return 0;
    *b = rbuf->buf[rbuf->lbound++];
    rbuf->lbound %= rbuf->size;
    rbuf->len--;
    if(rbuf->len <= 0) rbuf->flags |= RingBuffer_flag_Empty;
    if(rbuf->len < rbuf->size)rbuf->flags &= ~RingBuffer_flag_Full;
    interrupt_enable();
    return 1;
}