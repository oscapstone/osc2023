#ifndef	_MINI_UART_H
#define	_MINI_UART_H

void uart_init ( void );
char uart_recv ( void );
void uart_send ( char c );
void uart_send_string(char* str);
int uart_recv_line(char* buf, int maxline);

#endif  /*_MINI_UART_H */