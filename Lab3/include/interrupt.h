#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "gpio.h"

#define ENABLE_IRQS_1 ((volatile unsigned int *)(MMIO_BASE + 0x0000b210))

#define IRQ_PENDING_1 ((volatile unsigned int *)(MMIO_BASE + 0x0000b204))
#define IRQ_PENDING_1_AUX_INT (1 << 29)

#define CORE0_INT_SRC ((volatile unsigned int *)(0x40000060))
#define CORE0_INT_SRC_GPU (1 << 8)
#define CORE0_INT_SRC_TIMER (1 << 1)


#define UART_IRQ_PRIORITY 10
#define TIMER_IRQ_PRIORITY 5

void enable_uart_interrupt();
void enable_uart_tx_interrupt();
void disable_uart_tx_interrupt();
void enable_uart_rx_interrupt();
void disable_uart_rx_interrupt();
void uart_tx_interrupt_handler();
void uart_async_putc(char c);
void uart_rx_interrupt_handler();
char uart_async_getc();

void enable_interrupt();
void disable_interrupt();
void irq_handler(unsigned long long x0);
void invalid_exception_handler(unsigned long long x0);
void sync_el0_64_handler();
void set_cpacr_el1();

#endif