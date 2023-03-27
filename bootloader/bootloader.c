#include "bootloader.h"
#include "uart.h"
#include "helper.h"

unsigned int get_kernel_size()
{
    unsigned int size = uart_read() << 24;
    size |= uart_read() << 16;
    size |= uart_read() << 8;
    size |= uart_read();

    uart_puts("Size: ");
    uart_hex(size);
    uart_puts("\n");

    return size;
}

void load_kernel(char *argv)
{
    uart_puts("Start loading kernel...\n");
    uart_puts("Waiting for a kernel...\n");

    char *kernel = KERNEL_BASE;

    unsigned int size = get_kernel_size();

    for (int i = 0; i < size; i++)
    {
        kernel[i] = uart_read();
    }

    uart_puts("Kernel loaded!\n");
    uart_puts("Booting...\n");

    ((void (*)(char))kernel)(argv);
}