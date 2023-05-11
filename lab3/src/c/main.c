#include "shell.h"
#include "uart.h"
#include "printf.h"
#include "mm.h"
#include "timer.h"
#include "exception.h"
int main()
{
    init_uart();
    // mini_uart_interrupt_enable();
    init_printf(0, putc);
    init_memory();
    init_timer();

    printf("Hello World!\n\n");

    shell_start();

    return 0;
}