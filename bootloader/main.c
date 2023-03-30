#include "uart.h"
#include "bootloader.h"

void main(char *argv)
{
    uart_init();

    load_kernel(argv);
}
