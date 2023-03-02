#include "mini_uart.h"
#include "shell.h"

int main(void)
{
	uart_init();
	uart_send_string("Hello, world!\r\n");

	// while (1) {
	// 	uart_send(uart_recv());
	// }

	shell_start();

	return 0;
}
