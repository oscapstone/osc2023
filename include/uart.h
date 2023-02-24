#ifndef	_UART_H
#define	_UART_H
extern void uart_init(void);
extern char uart_read(void);
extern unsigned int uart_readline(char *buffer, unsigned int buffer_size);
extern void uart_write(char c);
extern void uart_write_string(char *str);
extern void uart_write_no(unsigned int n);
extern void uart_write_no_hex(unsigned int n);
#endif