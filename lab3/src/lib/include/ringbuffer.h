#ifndef __RINGBUFFER__
#define __RINGBUFFER__

#include <stdint.h>
#include <stddef.h>

typedef struct{
    uint32_t flags;
    char *buf;
    uint32_t lbound,rbound;
    size_t size, len;
} RingBuffer;

#define RingBuffer_flag_Empty 0b1
#define RingBuffer_flag_Full 0b10
#define RingBuffer_Empty(buf) (buf->flags&RingBuffer_flag_Empty)
#define RingBuffer_Full(buf) (buf->flags&RingBuffer_flag_Full)

RingBuffer *RingBuffer_new(size_t bufsize);
size_t RingBuffer_writeb(RingBuffer *rbuf, char b);
size_t RingBuffer_readb(RingBuffer *rbuf, char* b);

#endif