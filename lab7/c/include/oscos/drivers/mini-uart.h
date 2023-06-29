#ifndef OSCOS_DRIVERS_MINI_UART_H
#define OSCOS_DRIVERS_MINI_UART_H

void mini_uart_init(void);

int mini_uart_recv_byte_nonblock(void);
int mini_uart_send_byte_nonblock(unsigned char b);

void mini_uart_enable_rx_interrupt(void);
void mini_uart_disable_rx_interrupt(void);
void mini_uart_enable_tx_interrupt(void);
void mini_uart_disable_tx_interrupt(void);

#endif
