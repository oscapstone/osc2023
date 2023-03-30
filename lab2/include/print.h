#ifndef _PRINT_H
#define _PRINT_H

#include "type.h"

void putc(char);
void print(char *);
void printhex(unsigned int, bool);
void printf(char *str, ...);
void uart_send_int(int);

#endif
