#ifndef __STRING__
#define __STRING__
#include <stdint.h>
#include <stddef.h>
int atoi(const char* str);
int strcmp(const char* x, const char* y);
int strncmp(const char*x, const char*y, int len);
size_t strlen(const char *str);
void *memcpy(void *dst, const void *src, size_t n);
#endif