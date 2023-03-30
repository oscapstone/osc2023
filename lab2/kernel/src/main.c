#include "uart.h"
#include "utils.h"

//extern char* dtb;
int main(char* arg) { 
	char *dtb = arg; //get dtb from bootloader
	uart_send_string("print dtb address\n");
	uart_hex(&arg);
	uart_send_string("\nprint dtb\n");
	uart_hex(arg);
	uart_send_string("print dtbdtb:\n")
	uart_hex(dtb);
	shell_init();
	char cmd[100];
	int status=0;
	while (1) {
		switch (status) {
			case 0:
				shell_input(cmd);
				status = 1;
				break;
			case 1:
				shell_command(cmd);
				status = 0;
				break;
		}
	}
}


