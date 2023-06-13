#include "oscos/libc/stdio.h"

#include "oscos/utils/fmt.h"

int snprintf(char str[const restrict], const size_t size,
             const char *const restrict format, ...) {
  va_list ap;
  va_start(ap, format);

  const int result = vsnprintf(str, size, format, ap);

  va_end(ap);
  return result;
}

typedef struct {
  char *str;
  size_t size, len;
} snprintf_arg_t;

static void _snprintf_putc(const unsigned char c, snprintf_arg_t *const arg) {
  if (arg->size > 0 && arg->len < arg->size - 1) {
    arg->str[arg->len++] = c;
  }
}

static void _snprintf_finalize(snprintf_arg_t *const arg) {
  if (arg->size > 0) {
    arg->str[arg->len] = '\0';
  }
}

static const printf_vtable_t _snprintf_vtable = {
    .putc = (void (*)(unsigned char, void *))_snprintf_putc,
    .finalize = (void (*)(void *))_snprintf_finalize};

int vsnprintf(char str[const restrict], const size_t size,
              const char *const restrict format, va_list ap) {
  snprintf_arg_t arg = {.str = str, .size = size, .len = 0};
  return vprintf_generic(&_snprintf_vtable, &arg, format, ap);
}
