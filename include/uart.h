#ifndef __UART__
#define __UART__
void uart_init();
void uart_send(unsigned int c);
char uart_getc();
void uart_puts(char *s);
void uart_hex(unsigned int d);
void uart_getline(char* buf, int maxlen);
int uart_get_int();
void uart_send_int(int number);
char uart_recv();

void async_uart_recv();
void async_uart_puts();

void enable_uart_irq();
void disable_uart_irq();

int async_uart_handle();
#endif