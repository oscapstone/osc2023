#include "uart.h"
#include "mbox.h"
#include "shell.h"
#include "string.h"

void main()
{
    // set up serial console
    uart_init();

    shell();
}
