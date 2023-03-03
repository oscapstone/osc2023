#ifndef __GPIO_H__
#define __GPIO_H__

#include <stdarg.h>

#include "lib.h"
#include "peripherals/gpio.h"
#include "peripherals/uart.h"

void uart_init();
void uart_putchar(unsigned int);
char uart_getchar();
void uart_puts(const char*);
void uart_printf(const char*, ...);

#endif
