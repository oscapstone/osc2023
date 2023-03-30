#include <mem.h>
#include <mini_uart.h>

char start_mem;
char end_mem;

#define SMEM (&start_mem)
#define EMEM (&end_mem)

static char *cur = SMEM;

void *simple_malloc(uint32 size){
    char *tmp;

    if((uint64)cur + size > (uint64)EMEM){
        uart_printf("[!] No enough space!\r\n");
        return 0;
    }

    tmp = cur;
    cur += size;

    return (void*)tmp;
}