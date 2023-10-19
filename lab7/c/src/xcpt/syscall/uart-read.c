#include "oscos/console-suspend.h"

ssize_t sys_uart_read(char buf[const], const size_t size) {
  return console_read_suspend(buf, size);
}
