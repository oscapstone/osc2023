#include "oscos/console-suspend.h"

size_t sys_uart_write(const char buf[const], const size_t size) {
  return console_write_suspend(buf, size);
}
