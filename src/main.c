#include "mini_uart.h"
#include "shell.h"
#include "peripherals/mailbox.h"
#define MAX_CMD 64

enum stat {
	read,
	parse
};

void kernel_main(void)
{
	uart_init();
	uart_puts("OSC 2023 shell fron Stefan, please type: \n");
	
	enum stat s = read;
	char *cmd[MAX_CMD];
	buf_clear(cmd);
	while (1) {
		if(s == read){
			uart_puts("# ");
			shell_input(cmd);
			s = parse;
		} else {
			parse_cmd(cmd);
			s = read;
		}
	}
}
