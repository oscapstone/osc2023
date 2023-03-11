#include "muart.h"
#include "utils.h"

#define SIZE 64

void bootloader(void) {
    mini_uart_init();

    while (1) {
        char buffer[SIZE];
        mini_uart_gets(buffer, SIZE);
        if (strcmp(buffer, "loadimg") != 0) {
            mini_uart_puts("Please type \"loadimg\" to load the kernel image\r\n");
        } else {
            break;
        }
    }

    mini_uart_puts("\r\nBootloader starts obtaining the file size ...\r\n");

    char *kernel_location = (char*) 0x80000;
    unsigned long int kernel_size = 0;

    for (int i = 0; i < 4; i++) {
        kernel_size <<= 8;
        kernel_size |= mini_uart_getc();
    }

    mini_uart_puts("Bootloader starts receiving the kernel img ...\r\n");

    for (unsigned int i = 0; i < kernel_size; i++) {
        kernel_location[i] = mini_uart_getc();
    }

    void (*start_kernel)(void) = (void*) kernel_location;

    start_kernel();

    return;
}