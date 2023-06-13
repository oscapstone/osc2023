#include "uart.h"
#include "shell.h"

int bootloader()
{
    // set up serial console
    uart_init();
    
    // Welcome Message
    welcome_msg();

    // start shell
    shell_start();

    return 0;
}