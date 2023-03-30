#ifndef	_MINI_UART_H
#define	_MINI_UART_H

void uart_init (void);
char uart_recv (void);
unsigned int uart_recv_int (void);
void uart_gets(char*);
void uart_send (char);
void uart_puts(char*);
void uart_hex(unsigned int);
void uart_ptr(void*);

#endif  /*_MINI_UART_H */
