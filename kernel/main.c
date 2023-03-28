#include "uart.h"
#include "shell.h"
#include "devicetree.h"
#include "cpio.h"

void main(char *argv)
{
    uart_init();

    dt_tranverse(argv, INITRD_START, init_cpio);

    start_shell();
}
