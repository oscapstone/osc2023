#include "kernel.h"
#include "uart.h"
#include "helper.h"

void load_kernel()
{
    uart_puts("Start loading kernel...\n");
    uart_puts("Waiting for a kernel...\n");

    unsigned int *kernel = KERNEL_BASE;
    
    // read size of kernel
    unsigned int size = uart_read() << 24;
    size |= uart_read() << 16;
    size |= uart_read() << 8;
    size |= uart_read();

    uart_puts("Size: ");
    uart_hex(size);
    uart_puts("\n");

    while(size--) {
        *kernel++ = uart_read();
    }

    uart_puts("Kernel loaded!\n");
    uart_puts("Booting...\n");

    ((void(*)(void))0x8000)();

    wait_cycles(10000);   
    uart_puts("You should not be here!");
}