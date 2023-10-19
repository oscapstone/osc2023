#ifndef OSCOS_LIBC_STDIO_H
#define OSCOS_LIBC_STDIO_H

#include <stdarg.h>
#include <stddef.h>

int snprintf(char str[restrict], size_t size, const char *restrict format, ...)
    __attribute__((format(printf, 3, 4)));
int vsnprintf(char str[restrict], size_t size, const char *restrict format,
              va_list ap) __attribute__((format(printf, 3, 0)));

#endif
