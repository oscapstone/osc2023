#include "mini_uart.h"

#define HEAP_SIZE 0x10000

extern char heap_start;

static char *heap_tail = &heap_start;
static int heap_current_size = 0;

void* simple_malloc(unsigned int size)
{
        if (heap_current_size + size > HEAP_SIZE) {
                uart_send_string("[ERROR] exceed max heap size\r\n");
                return (char*) 0;
        }
        char* adr = heap_tail;
        heap_current_size += size;
        heap_tail += size;
        return adr;
}

void show_heap_size(void)
{
        uart_send_string("Current heap size: ");
        uart_send_int(heap_current_size);
        uart_endl();
}