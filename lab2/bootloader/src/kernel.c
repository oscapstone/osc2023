#include "mini_uart.h"
void kernel_main()
{
	uart_init();
	char *kernel = (char*)0x80000;
	uart_send_string("bootloader start\r\n");

	unsigned int size = 0;
	char c;
	while(1)
	{
		if(size > 1000000)		//ghost story
		{
			size = 0;
		}
		c = uart_recv_ker();
		if(c == ';')
		{
			break;
		}
		int n = c-'0';
		size *= 10;
		size += n;
	}
	uart_int(size);
	uart_send_string("\r\n");
	
	while(size--)
	{
		c = uart_recv_ker();
		*kernel = c;
		uart_send('>');
		uart_int(size);
		kernel++;
	}
	uart_send_string("success!!\r\n");
	
	asm volatile(
		"mov x0,x27;"
		"mov x30,#0x80000;"
		"ret"	
	);
	return;
}
