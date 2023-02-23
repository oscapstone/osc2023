#include "uart.h"
#include "str.h"
#include "terminal.h"

int main(void){
	uart_setup();
	terminal_run();
	/*
	uart_puts("Hello world!\n");
	while(1){
		uart_send(uart_getc());
	}
	*/
	return 0;
}
