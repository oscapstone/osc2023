#include "bcm2835/uart.h"

static void kernel_init() {
    uart_init();
}

void start_kernel() {
    kernel_init();
    uart_puts("Hello World!\n");
    do {
        uart_send(uart_recv());
    } while (1);
}