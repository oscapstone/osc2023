#ifndef UART_H
#define UART_H

#define MAX_BUF_SIZE 0x100

void uart_init();
void uart_putc(char c);
char uart_getc();
int uart_puts(char *s);
char* uart_gets(char *buf);
int uart_printf(char *s);
void uart_hex(unsigned int d);
void disable_uart();

#endif
