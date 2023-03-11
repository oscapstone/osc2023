#ifndef _UART_H
#define _UART_H

volatile unsigned int  __attribute__((aligned(16))) mbox[36];

void uart_init ( void );
void uart_send(char c);
char uart_getc();
void uart_puts(char *s);
void uart_hexdump(unsigned int d);
void uart_hex(unsigned int d);

#endif