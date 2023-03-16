#include "allocator.h"
#include "utils_c.h"
#include "mini_uart.h"

#define MEM_SIZE 0x10000000 // 0.25G
#define MEM_START 0x10000000

// unsigned long *malloc_cur = (unsigned long *)MEM_START;
unsigned char *malloc_cur = (unsigned char *)MEM_START;

void *malloc(size_t size)
{
    uart_send_string("Before the buffer pointer is at: ");
    uart_hex((long unsigned int *)malloc_cur);
    uart_send_string("\n");
    // unsigned int temp = (unsigned long int *)malloc_cur;
    // unsigned int *print_addr = (unsigned int*)malloc_cur;
    // printf("%d\n", malloc_cur);
    // uart_send_string(malloc_cur);

    align(&size, 4); // allocated the memory size is mutiple of 4 byte;
    unsigned char *malloc_ret = malloc_cur;
    // malloc_cur += (unsigned int)size;
    malloc_cur += size;
    uart_send_string("Now the buffer pointer is at: ");
    uart_hex((long unsigned int *)malloc_cur);
    uart_send_string("\n");
    // printf("%d\n", malloc_cur);
    return malloc_ret;
}