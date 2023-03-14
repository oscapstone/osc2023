#ifndef _STDLIB_H
#define _STDLIB_H

#include <stdint.h>
#include <stddef.h>

int strcmp(const char *str1, const char *str2);
int strlen(const char *str);

void *memset(void *dest, register int val, int len);
int memcmp(void *s1, void *s2, int n);
int hex2int(char *s, int n);

void *simple_malloc(unsigned int size);

void printf(char *fmt, ...);
unsigned int sprintf(char *dst, char *fmt, ...);
unsigned int vsprintf(char *dst, char *fmt, __builtin_va_list args);

uint32_t align_up(uint32_t size, int alignment);

#endif /*_STDLIB_H */
