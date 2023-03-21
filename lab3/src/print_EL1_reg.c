#include "mini_uart.h"

int exception_entry()
{
	char* spsr_el1;
	char* elr_el1;
	char* esr_el1;
	asm volatile
	(
		 "mrs	%0,SPSR_EL1;"
		 "mrs	%1,ELR_EL1;"
		 "mrs	%2,ESR_EL1;" : "=r" (spsr_el1) , "=r" (elr_el1) , "=r" (esr_el1)
	);
	uart_send_string("SPSR_EL1 : ");
	uart_hex(spsr_el1);
	uart_send_string("\r\n");
	uart_send_string("ELR_EL1 : ");
	uart_hex(elr_el1);
	uart_send_string("\r\n");
	uart_send_string("ESR_EL1 : ");
	uart_hex(esr_el1);
	uart_send_string("\r\n");
	return 0;
}
