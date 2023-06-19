#ifndef OSCOS_USER_PROGRAM_LIBC_STDIO_H
#define OSCOS_USER_PROGRAM_LIBC_STDIO_H

#include <stdarg.h>

int printf(const char *restrict format, ...)
    __attribute__((format(printf, 1, 2)));

int vprintf(const char *restrict format, va_list ap)
    __attribute__((format(printf, 1, 0)));

#endif
