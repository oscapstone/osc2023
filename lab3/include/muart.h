#ifndef __MUART_H__
#define __MUART_H__

#include "mmio.h"

#define AUX_ENABLE      ((volatile unsigned int*)(MMIO_BASE + 0x215004))
#define AUX_MU_IO_REG   ((volatile unsigned int*)(MMIO_BASE + 0x215040))
#define AUX_MU_IER_REG  ((volatile unsigned int*)(MMIO_BASE + 0x215044))
#define AUX_MU_IIR_REG  ((volatile unsigned int*)(MMIO_BASE + 0x215048))
#define AUX_MU_LCR_REG  ((volatile unsigned int*)(MMIO_BASE + 0x21504C))
#define AUX_MU_MCR_REG  ((volatile unsigned int*)(MMIO_BASE + 0x215050))
#define AUX_MU_LSR_REG  ((volatile unsigned int*)(MMIO_BASE + 0x215054))
#define AUX_MU_CNTL_REG ((volatile unsigned int*)(MMIO_BASE + 0x215060))
#define AUX_MU_BAUD     ((volatile unsigned int*)(MMIO_BASE + 0x215068))

void delay(int waits);
void mini_uart_init(void);
char mini_uart_getc(void);
void mini_uart_gets(char *buffer, int size);
void mini_uart_putc(char  c);
void mini_uart_puts(char *s);

#endif