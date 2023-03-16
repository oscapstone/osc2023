#include "mini_uart.h"
#include "shell.h"
#include "device_tree.h"

void kernel_main(void)
{
	uart_init();
	uart_send_string("Kernel Starts...\r\n");
	get_device_tree_adr();
	shell_loop();
}