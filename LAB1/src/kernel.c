#include "mini_uart.h"
#include "shell.h"
#include "peripherals/reboot.h"
#include "mbox.h"
char input_ch;

void kernel_main(void)
{
	uart_init();
	mbox_run();
	while (1)
	{
		input_ch = uart_recv();
		uart_send(input_ch);
		shell_run(input_ch);
	}
}
