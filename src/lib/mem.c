#include <mem.h>
#include <mini_uart.h>
#include <type.h>

static char *cur = SMEM;

void *simple_malloc(uint32 size){
    char *tmp;
    size = ALIGN(size, 4);
    if((uint64)cur + size > (uint64)EMEM){
        uart_printf("[!] No enough space!\r\n");
        return 0;
    }

    tmp = cur;
    cur += size;
#ifdef DEBUG
    uart_printf("[*] Early allocate: %llx ~ %llx\r\n", tmp, cur - 1);
#endif
    return (void*)tmp;
}