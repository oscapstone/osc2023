#include "mini_uart.h"
#include "mem_frame.h"

#define HEAP_SIZE 0x1000000

extern char heap_start;

static char *heap_tail = &heap_start;
static int heap_current_size = 0;

void* startup_allocator(unsigned int size)
{
        if (heap_current_size + size > HEAP_SIZE) {
                uart_send_string("[ERROR] exceed max heap size\r\n");
                return (char*) 0;
        }
        char* adr = heap_tail;
        heap_current_size += size;
        heap_tail += size;
        memory_reserve(adr, heap_tail);
        return adr;
}

void show_heap_size(void)
{
        uart_send_string("Current heap size: ");
        uart_send_int(heap_current_size);
        uart_endl();
}

void* memcpy(void *dest, const void *src, int len)
{
        char *d = dest;
        const char *s = src;
        while (len--) *d++ = *s++;
        return dest;
}