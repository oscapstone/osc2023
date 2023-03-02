#include "mini_uart.h"
#include "shell.h"
#include "mbox.h"

#define COMMAND_BUFFER 20

void kernel_main(void)
{
	char c;
	int i;
	char command[COMMAND_BUFFER];

	uart_init();

	mbox_main();

	uart_send_string("Starting shell...\n");
	uart_send_string("# ");

	while (1)
	{
		c = uart_recv();

		if (c == '\n')
		{
			command[i++] = '\0';
			shell_main(command);
			i = 0;
			uart_send(c);
			uart_send_string("# ");
		}
		else
		{
			if (i < COMMAND_BUFFER)
				command[i++] = c;
			uart_send(c);
		}
	}
}
