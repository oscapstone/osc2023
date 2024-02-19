#ifndef _STDLIB_H
#define _STDLIB_H

#include <stdint.h>
#include <stddef.h>
#include "printf.h"
#include "utils.h"
#include "mini_uart.h"
#include "page_alloc.h"
#include "dynamic_alloc.h"

int strcmp(const char *str1, const char *str2);
int strlen(const char *str);
char *strcpy(char *destination, const char *source);
int atoi(char *str);

void *memset(void *dest, register int val, int len);
int memcmp(void *s1, void *s2, int n);
void *memcpy(void *dest, const void *src, size_t len);
int hex2int(char *s, int n);

void *simple_malloc(unsigned int size);

// void printf(char *fmt, ...);
// unsigned int sprintf(char *dst, char *fmt, ...);
// unsigned int vsprintf(char *dst, char *fmt, __builtin_va_list args);

#endif /*_STDLIB_H */
