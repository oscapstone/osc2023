#ifndef	MINI_UART_H
#define	MINI_UART_H

void uart_init(void);
char uart_recv(void);
void uart_send(char c);

void uart_send_string(char* str);
void uart_send_hex_32(unsigned int n);
void uart_send_hex_64(unsigned long n);
int  uart_recv_int(void);
void uart_send_int(int num);
void uart_endl(void);
void uart_readline(char* buffer, int len);

#endif /* MINI_UART_H */