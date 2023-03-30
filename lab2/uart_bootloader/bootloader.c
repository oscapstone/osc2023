#include "mini_uart.h"

void bootloader() {
    
    uart_init();
    uart_send_string("\r\nInput \"s\" to start bootloader\r\n");

    while (1) {
        char c = uart_recv();
        uart_send(c);
        if (c == 's') break;
    }

    uart_send_string("\r\nUART bootloader\r\n");

    char *kernel_location = (char *)0x80000;
    unsigned int kernel_size = 0;

    for (int i=0; i<4; i++) {
        kernel_size <<= 8;
        kernel_size |= uart_recv();
    }

    uart_send_string("file size obtained\r\n");
    uart_send_string("now receiving img...\r\n");

    for (unsigned int i=0; i<kernel_size; i++) {
        kernel_location[i] = uart_recv();
    }

    uart_send_string("Image received.Starting kernel...\r\n");
    uart_send_string("\r\n");

    void (*start_kernel)(void) = (void *)kernel_location;

    start_kernel();

    return;
}
