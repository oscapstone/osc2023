#include "dtb.h"
#include "uart.h"
#include "shell.h"
extern char* dtb_base;

void main(char *arg)
{
    dtb_base = arg;
    fdt_traverse(initramfs_callback);
    uart_init();
    shell();
}