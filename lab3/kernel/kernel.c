#include "mini_uart.h"
#include "shell.h"
#include "device_tree.h"
#include "exception.h"

void kernel_main(void)
{
	set_exception_vector_table();
	uart_init();
	uart_send_string("Kernel Starts...\r\n");
	get_device_tree_adr();
	shell_loop();
}