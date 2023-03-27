#include "mini_uart.h"

void print_time()
{
	char* time;
	char* freq;
	asm volatile
	(
		"mrs %0,CNTPCT_EL0;"
		"mrs %1,CNTFRQ_EL0;" : "=r" (time) , "=r" (freq)
	);
	uart_send_string("core timeout interrupt : ");
	uart_hex((unsigned int)time / (unsigned int)freq);		//second : time/freq
	uart_send_string(" s\r\n");
	return;
}
