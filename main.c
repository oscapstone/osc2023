#include "uart.h"
#include "shell.h"
#include "kernel.h"

void main()
{
    uart_init();
    uart_puts("Bootloader loaded!\n");

    load_kernel();
}
