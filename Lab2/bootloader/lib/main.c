#include "uart.h"
#include "shell.h"
#include "load.h"
#include "relocate.h"

int relocated = 1;
extern char* dtb_base;

void main(char *arg)
{
    dtb_base = arg;
    if (relocated)
    {
        relocated = 0;
        relocate(arg);
    }

    uart_init();
    shell();
}