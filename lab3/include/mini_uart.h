#ifndef	_MINI_UART_H
#define	_MINI_UART_H

void uart_init (void);
char uart_recv (void);
unsigned int uart_recv_int (void);
void uart_gets(char*);
void uart_send (char);
void uart_puts(char*);
void uart_hex(unsigned int);
void uart_dec(unsigned int);
void uart_ptr(void*);

void enable_uart_interrupt();
void disable_uart_interrupt();

void enable_read_interrupt();
void disable_read_interrupt();
void enable_write_interrupt();
void disable_write_interrupt();

void async_uart_handler();
void async_uart_puts(const char *s);
unsigned int async_uart_gets(char *buffer, unsigned int size);


#endif  /*_MINI_UART_H */
