#ifndef UART_ASYNC_H
#define UART_ASYNC_H

int uart_async_send_string(char* str);
int uart_async_readline(char* target, int len);
void demo_uart_async(void);

void uart_async_interrupt_handler(void);

#endif /* UART_ASYNC_H */