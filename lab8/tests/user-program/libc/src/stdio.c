#include "stdio.h"

#include "__detail/utils/fmt.h"
#include "unistd.h"

static void _printf_putc(const unsigned char c, void *const _arg) {
  (void)_arg;

  // Very inefficient, but it works.
  while (uart_write(&c, 1) != 1)
    ;
}

static void _printf_finalize(void *const _arg) { (void)_arg; }

static const printf_vtable_t _printf_vtable = {.putc = _printf_putc,
                                               .finalize = _printf_finalize};

int printf(const char *const restrict format, ...) {
  va_list ap;
  va_start(ap, format);

  const int result = vprintf(format, ap);

  va_end(ap);
  return result;
}

int vprintf(const char *const restrict format, va_list ap) {
  return __vprintf_generic(&_printf_vtable, NULL, format, ap);
}
