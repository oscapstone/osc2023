#ifndef STRING_H
#define STRING_H
#include "stdint.h"

int strncmp(const char *str1, const char *str2, uint32_t len);
char* strncpy(char *dst, const char *src, uint32_t len);
void* memset(char *dst, char c, uint32_t len);
int strcmp(char *a, char *b);
#endif
