#ifndef UART_H
#define UART_H

#include "gpio.h"
#include "sprintf.h"
#include "exception.h"

#define MAX_BUF_SIZE 0x400

/* Auxilary mini UART registers */
#define AUX_ENABLE ((volatile unsigned int *)(MMIO_BASE + 0x00215004))
#define AUX_MU_IO ((volatile unsigned int *)(MMIO_BASE + 0x00215040))
#define AUX_MU_IER ((volatile unsigned int *)(MMIO_BASE + 0x00215044))
#define AUX_MU_IIR ((volatile unsigned int *)(MMIO_BASE + 0x00215048))
#define AUX_MU_LCR ((volatile unsigned int *)(MMIO_BASE + 0x0021504C))
#define AUX_MU_MCR ((volatile unsigned int *)(MMIO_BASE + 0x00215050))
#define AUX_MU_LSR ((volatile unsigned int *)(MMIO_BASE + 0x00215054))
#define AUX_MU_MSR ((volatile unsigned int *)(MMIO_BASE + 0x00215058))
#define AUX_MU_SCRATCH ((volatile unsigned int *)(MMIO_BASE + 0x0021505C))
#define AUX_MU_CNTL ((volatile unsigned int *)(MMIO_BASE + 0x00215060))
#define AUX_MU_STAT ((volatile unsigned int *)(MMIO_BASE + 0x00215064))
#define AUX_MU_BAUD ((volatile unsigned int *)(MMIO_BASE + 0x00215068))

void uart_init();
void enable_mini_uart_interrupt();
void enable_mini_uart_rx_interrupt();
void enable_mini_uart_tx_interrupt();
void disable_mini_uart_interrupt();
void disable_mini_uart_rx_interrupt();
void disable_mini_uart_tx_interrupt();

/* UART interrupt handler */
void uart_rx_interrupt_handler(); 
void uart_tx_interrupt_handler();

/* Asynchronous UART */
void uart_async_putc(char c);
char uart_async_getc();
char *uart_async_gets(char *buf);
int uart_async_printf(char *fmt, ...);

/* Synchronous UART */
void uart_putc(char c);
void uart_puts(char *s);
char uart_getc();
char *uart_gets(char *buf);
int uart_printf(char *fmt, ...);

#endif