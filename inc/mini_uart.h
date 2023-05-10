#ifndef	_MINI_UART_H
#define	_MINI_UART_H
#include <type.h>

char uart_recv (void);
void uart_send (char c);
void uart_send_string(char *str);
void uart_send_hex(unsigned int d);
int uart_recv_line(char *buf, int maxline);
uint32 uart_recv_uint();
void uart_printf(char *fmt, ...);
void uart_sendn(char *str, int n);
void uart_init (void);

int uart_irq_add(void);
void uart_irq_handler(void);
int uart_switch_mode(void);
#endif  /*_MINI_UART_H */