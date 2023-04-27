#ifndef UART_H
#define UART_H

#include "peripherals/rpi_uart.h"

void uart_init();
void uart_send(unsigned int c);
char uart_getc();
void uart_puts(char *s);
int uart_get_int();

#endif