#ifndef	_UART_H
#define	_UART_H
extern void uart_init(void);
extern char _uart_read(void);
extern char uart_read(void);
extern unsigned int uart_readline(char *buffer, unsigned int buffer_size);
extern unsigned long long uart_read_hex_ull();
extern void uart_write(char c);
extern void uart_write_string(char *str);
extern void uart_write_no(unsigned long long n);
extern void uart_write_no_hex(unsigned long long n);
extern void dump_hex(const void* ptr, unsigned long long size) ;
extern void uart_write_retrace();
extern unsigned int uart_read_input(char *cmd, unsigned int cmd_size);
#endif