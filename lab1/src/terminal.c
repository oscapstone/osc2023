#include "terminal.h"
#include "str.h"
#include "uart.h"
#include "mailbox.h"
#include "command.h"

#define BUF_LEN 256

void terminal_run(){
	char input_buf[BUF_LEN + 1];
	char* tmp;
	uart_puts("=======Terminal start!!======\n");

	while(1){
		tmp = input_buf;
		tmp[0] = 0;
		uart_putc('>');
		for(int i = 0; i < BUF_LEN; i++){
			*tmp = uart_getc();
			uart_putc(*tmp);

			if(*tmp == 127){
				*tmp = 0;
				tmp --;
				*tmp = 0;
				tmp --;
				uart_send('\b');
				uart_send(' ');
				uart_send('\b');
			}
			if(*tmp == '\n'){
				*(tmp) = '\0';
				break;
			}
			tmp ++;
		}

		command_exec(input_buf);
	}
}

// This implementation is slow but as for now the 
// number of commands is small.
int command_exec(const char* in){
	int i = 0;
	while(1){
		// If the cannot find the command, just return.
		if(!strcmp(commands[i].name, "NULL")){
			invalid_command(in);
			return 1;
		}
		// Target command found
		if(!strcmp(in, commands[i].name)){
			commands[i].func();
			return 0;
		}
		i++;
	}
	return 0;
}
