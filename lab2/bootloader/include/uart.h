#ifndef UART_H
#define UART_H

void uart_init();
void uart_write(unsigned int c);
char uart_read();
char uart_read_raw();
int uart_printf(char *fmt, ...);
void uart_printf_n(char *s, int len);
void uart_flush();
void uart_hex();

#endif