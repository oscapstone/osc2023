#ifndef MY_STR_H
#define MY_STR_H

#define VSPRINT_MAX_BUF_SIZE 128

unsigned int vsprintf(char *dst, char* fmt, __builtin_va_list args);
int strcmp(const char*, const char*);
char* memcpy(void *dest, const void *src, unsigned long long len);
int strncmp(const char*, const char*, unsigned long long);
unsigned long long strlen(const char *str);
#endif