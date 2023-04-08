#ifndef _UART_H_
#define _UART_H_


#include "peripherals/rpi_uart.h"


/* Set baud rate and characteristics (115200 8N1) and map to GPIO*/
void uart_init();

/* Uart feature */
void uart_send(unsigned int c);
char uart_getc();
void uart_puts(char *s);
int  uart_printf(char* fmt, ...);
int  uart_get_int();
void uart_puts_bySize(char *s, int size);
void uart_put_int(unsigned long num);
void uart_put_hex(unsigned long num);
/* Uart async feature */
void uart_async_putc(char c);
char uart_async_getc();
char uart_recv();
int  uart_async_puts(char* fmt, ...);
int  uart_async_printf(char *fmt, ...);

/* Uart interrupt controllers */
void uart_interrupt_enable();
void uart_interrupt_disable();
void uart_r_irq_handler();
void uart_w_irq_handler();

#endif