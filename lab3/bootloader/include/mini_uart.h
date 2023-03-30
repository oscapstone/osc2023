#ifndef MINI_UART_H

#define MINI_UART_H
void uart_init(void);
char uart_recv(void);
char uart_recv_ker(void);
void uart_send(char c);
void uart_send_string(char* str);
void uart_send_num_string(char* str,int n);
void uart_hex(unsigned int d);
void uart_int(unsigned int d);

#endif
