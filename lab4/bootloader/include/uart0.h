#ifndef UART0_H

#define UART0_H
#include "base.h"

#define UART0_DR		(volatile unsigned int*)(BASE+0x00201000)
#define UART0_FR		(volatile unsigned int*)(BASE+0x00201018)
#define UART0_IBRD		(volatile unsigned int*)(BASE+0x00201024)
#define UART0_FBRD		(volatile unsigned int*)(BASE+0x00201028)
#define UART0_LCRH		(volatile unsigned int*)(BASE+0x0020102C)
#define UART0_CR		(volatile unsigned int*)(BASE+0x00201030)
#define UART0_ICR		(volatile unsigned int*)(BASE+0x00201044)

void uart0_init();
void uart0_send(unsigned int c);
char uart0_recv();
void uart0_send_string(char *str);

#endif
