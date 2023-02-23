#include "uart.h"

int main(void){
	uart_setup();
	uart_puts("Hello world!\n");
	while(1){
		uart_send(uart_getc());
	}
	return 0;
}
