#ifndef	_UART_H
#define	_UART_H
#include "mini_uart.h"
#define BUFFER_MAX_SIZE 0x1000
#define UART_READABLE() (*AUX_MU_LSR & 0x01)
extern void uart_init(void);
extern char _kuart_read(void);
extern char kuart_read(void);
extern unsigned int uart_readline(char *buffer, unsigned int buffer_size);
extern unsigned long long uart_read_hex_ull();
extern void kuart_write(char c);
extern void _kuart_write(char c);
extern void uart_write_string(char *str);
extern void uart_write_no(unsigned long long n);
extern void uart_write_no_hex(unsigned long long n);
extern void uart_write_fraction(unsigned numerator, unsigned denominator, unsigned deg);
extern void dump_hex(const void* ptr, unsigned long long size) ;
extern void uart_write_retrace();
extern unsigned int uart_read_input(char *cmd, unsigned int cmd_size);
extern void uart_irq_handler();
#endif