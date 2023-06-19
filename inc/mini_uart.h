#ifndef	_MINI_UART_H
#define	_MINI_UART_H
#include <type.h>
#include <stdarg.h>

char uart_recv (void);
void uart_recvn(char *buff, int n);
void uart_send (char c);
void uart_printf(const char *fmt, ...);
void uart_sync_printf(const char *fmt, ...);
void uart_sync_vprintf(const char *fmt, va_list args);

int uart_recv_line(char *buf, int maxline);
uint32 uart_recv_uint();

void uart_sendn(const char *str, int n);
void uart_init (void);

int uart_irq_add(void);
int uart_switch_mode(void);
#endif  /*_MINI_UART_H */