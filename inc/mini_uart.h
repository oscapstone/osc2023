#ifndef	_MINI_UART_H
#define	_MINI_UART_H

void uart_send (char c);
void uart_send_string(char* str);
void uart_send_hex(unsigned int d);
char uart_recv (void);
int uart_recv_line(char* buf, int maxline);
void uart_init (void);

#endif  /*_MINI_UART_H */