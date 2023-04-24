#include "mini_uart.h"
#include "shell.h"
#include "devtree.h"
#include "cpio.h"
#include "exception.h"
#include "mm.h"

void kernel_main(void)
{
	uart_init();
	devtree_getaddr();
	fdt_traverse(initramfs_callback);
	init_mm_reserve();
	uart_send_string("OSDI 2023 Spring\n");
	uart_send_string("Welcome to simple shell!\n");
	uart_send_string("=========================\n");

	shell_loop();
}