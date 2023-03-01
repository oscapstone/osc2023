#include "bcm2835/uart.h"
#include "kernel/shell.h"

static void kernel_init() {
    uart_init();
}

void start_kernel() {
    kernel_init();
    uart_puts("Hello World!\n");
    shell_start();
}