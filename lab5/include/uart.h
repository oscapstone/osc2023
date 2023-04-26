#ifndef UART_H
#define UART_H

#include "gpio.h"
#include "uart.h"
#include "sprintf.h"
#include "registers.h"

#define MAX_BUF_SIZE 0x1000

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
void uart_disable_echo();
void uart_enable_echo();
void uart_putc(char c);
char uart_getc();
int uart_puts(char *s);
int uart_async_puts(char *s);
char* uart_gets(char *buf);
char* uart_async_gets(char *buf);
int uart_printf(char *fmt, ...);
int uart_async_printf(char *fmt, ...);
void disable_uart();
void uart_interrupt_r_handler();
void uart_interrupt_w_handler();
void enable_mini_uart_interrupt();
void enable_mini_uart_w_interrupt();
void enable_mini_uart_r_interrupt();
void disable_mini_uart_interrupt();
void disable_mini_uart_w_interrupt();
void disable_mini_uart_r_interrupt();
int mini_uart_r_interrupt_is_enable();
int mini_uart_w_interrupt_is_enable();

char uart_async_getc();
void uart_async_putc(char c);

#endif
