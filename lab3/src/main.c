#include "uart.h"
#include "shell.h"
#include "printf.h"
#include "mm.h"
#include "timer.h"

int main()
{
    // set up serial console
    uart_init();
    
    init_printf(0, putc);

    // Initialize memory allcoator
    mm_init();

    // Initialize timer list for timeout events
    timer_list_init();

    // start shell
    shell_start();

    return 0;
}
