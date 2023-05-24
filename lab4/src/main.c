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
    printf("pass any key start\n");
    uart_getc();

    void *address_1 = memory_allocation(16);
    void *address_2 = memory_allocation(64);
    printf("address_1: %x\n", address_1);
    printf("address_2: %x\n", address_2);
    memory_free(address_1);
    void *address_3 = memory_allocation(1024);
    printf("address_3: %x\n", address_3);
    memory_free(address_2);
    memory_free(address_3);
    void *address_4 = memory_allocation(16);
    printf("address_4: %x\n", address_4);
    memory_free(address_4);
    void *address_5 = memory_allocation(32);
    printf("address_5: %x\n", address_5);
    void *address_6 = memory_allocation(32);
    printf("address_6: %x\n", address_6);
    memory_free(address_5);
    memory_free(address_6);
    void *address_7 = memory_allocation(512);
    void *address_8 = memory_allocation(512);
    void *address_9 = memory_allocation(16384);
    printf("address_7: %x\n", address_7);
    printf("address_8: %x\n", address_8);
    printf("address_9: %x\n", address_9);
    memory_free(address_8);
    memory_free(address_7);
    void *address_10 = memory_allocation(8192);
    printf("address_10: %x\n", address_10);
    memory_free(address_9);
    memory_free(address_10);

    printf("DONE!!!!!\n");
    while (1)
    {
        shell_start();
    }

    return 0;
}