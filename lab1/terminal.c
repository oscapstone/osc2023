#include "terminal.h"
#include "str.h"
#include "uart.h"

#define BUF_LEN 256

void terminal_run(){
	char input_buf[BUF_LEN + 1];
	char* tmp;

	while(1){
		tmp = input_buf;
		for(int i = 0; i < BUF_LEN; i++){
			*tmp = uart_getc();
			uart_send(*tmp);
			if(*tmp == '\n'){
				*(tmp) = '\0';
				break;
			}
			tmp ++;
		}

		if(!strcmp(input_buf, "help"))
			help();
		else if(!strcmp(input_buf, "hello"))
			hello();
		//else if(!strcmp(input_buf, "reboot"))
			//reboot();
		else
			invalid_command();
	}
}
			
int help(){
	uart_puts(	
			"help\t: print this help message.\n"
			"hello\t: print Hello World!\n"
			"reboot\t: reboot the device.\n"
		 );
	return 0;
}

int hello(){
	uart_puts("Hello World!\n");
	return 0;
}

int invalid_command(){
	uart_puts("Invalid command! Please use `help` to list commands\n");
	return 0;
}
