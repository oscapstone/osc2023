#include "mini_uart.h"
#include "shell.h"

void kernel_main(void)
{
	uart_init();
	uart_send_string("Kernel Starts...\r\n");
	shell_loop();
}