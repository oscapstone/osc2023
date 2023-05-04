#include "shell.h"
#include "uart.h"
#include "printf.h"
#include "memory.h"

int main()
{
    uart_init();
    init_printf(0, putc);
    printf("Hello World!\n\n");

    printf("--------------------\n\n");

    init_memory();
    memory_reserve((void *)0x0, (void *)0x1000);

    void *address_1 = memory_allocation(16);
    void *address_2 = memory_allocation(64);
    printf("address_1: %x\n", address_1);
    memory_free(address_1);
    void *address_3 = memory_allocation(1024);
    memory_free(address_2);
    memory_free(address_3);
    void *address_4 = memory_allocation(16);
    memory_free(address_4);
    void *address_5 = memory_allocation(32);
    void *address_6 = memory_allocation(32);
    memory_free(address_5);
    memory_free(address_6);
    void *address_7 = memory_allocation(512);
    void *address_8 = memory_allocation(512);
    void *address_9 = memory_allocation(16384);
    memory_free(address_8);
    memory_free(address_7);
    void *address_10 = memory_allocation(8192);
    memory_free(address_9);
    memory_free(address_10);

    printf("DONE!!!!!\n");
    while (1)
    {
        shell_start();
    }

    return 0;
}