#include "oscos/panic.h"

#include "oscos/console.h"

void panic_begin(const char *const restrict file, const int line,
                 const char *const restrict format, ...) {
  // Prints the panic message.

  va_list ap;
  va_start(ap, format);

  console_printf("panic: %s:%d: ", file, line);
  console_vprintf(format, ap);
  console_putc('\n');

  va_end(ap);

  // Park the core.
  for (;;) {
    __asm__ __volatile__("wfe");
  }
}
