#ifndef	_MINI_UART_H
#define	_MINI_UART_H

void uart_init ( void );
char uart_recv ( void );
void uart_send ( char c );
void uart_send_string ( char* str );
void async_uart_handler ();
void async_uart_test ();

#endif  /*_MINI_UART_H */