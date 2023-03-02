#include "stdlib.h"
#include "mini_uart.h"
#include "reboot.h"

void shell_main(char *command)
{
    if (!strcmp(command, "help"))
    {
        uart_send_string("\n");
        uart_send_string("help\t: print this help menu\n");
        uart_send_string("hello\t: print Hello World!\n");
        uart_send_string("reboot\t: reboot the device");
    }
    else if (!strcmp(command, "hello"))
    {
        uart_send_string("\n");
        uart_send_string("Hello World!");
    }
    else if (!strcmp(command, "reboot"))
    {
        // TODO: Reboot
        uart_send_string("\n");
        uart_send_string("Rebooting in 3 seconds");
        reset(3 << 16);
    }
}
