#include "mini_uart.h"
#include "shell.h"
#include "utils.h"

int main()
{

	char c;
	unsigned int len;
	char *ptr = (char*) 0x80000;

	uart_init();

	uart_puts("Hello, I'm bootloader\r\n");
	
	while(1){
		uart_puts("Press L to start loading kernel image...\r\n");
		c = uart_recv();
		if(c == 'l' || c == 'L'){
            break;
        }
	}
	uart_puts("Start loading...\r\n");


	// while(1){
	// 	c = uart_recv();
	// 	uart_send(c);
	// 	continue;
	// }

	len = uart_recv_int();
	uart_puts("Kernel size: ");
	uart_hex(len);
	uart_puts("\r\n");

	for(unsigned int i=0;i<len;i++){
		ptr[i] = uart_recv();
		// uart_hex(i);
		// uart_puts("  ");
		// uart_hex(len);
		// uart_puts("\r\n");
	}
	
	uart_puts("Redirection to kernel\r\n\r\n");
	delay(4000);

	void (*start_kernel)() = (void*) ptr;

    start_kernel();

	return 0;
}
