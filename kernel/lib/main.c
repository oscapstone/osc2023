#include "shell.h"
#include "dtb.h"

void main(char *arg)
{
    dtb_base = arg;
    uart_init();

    welcome();
    fdt_traverse(initramfs_callback);
    shell();
}