#include "uart.h"
#include "shell.h"

void main(char *argv)
{
    uart_init();

    start_shell();
}
