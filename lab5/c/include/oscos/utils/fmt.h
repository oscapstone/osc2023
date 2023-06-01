#ifndef OSCOS_UTILS_FMT_H
#define OSCOS_UTILS_FMT_H

#include <stdarg.h>

typedef struct {
  void (*putc)(unsigned char, void *);
  void (*finalize)(void *);
} printf_vtable_t;

int vprintf_generic(const printf_vtable_t *vtable, void *arg,
                    const char *restrict format, va_list ap)
    __attribute__((format(printf, 3, 0)));

#endif
