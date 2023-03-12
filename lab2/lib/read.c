#include "read.h"
#include "type.h"
#include "print.h"
#include "mini_uart.h"

int readline(char *buf, int len, bool interact) {
  char ch;
  int i = 0;
  for (i = 0; i < len; i++) {
    ch = uart_recv();
    if (interact == true)
      putc(ch);
    if (ch == '\n' || ch == '\r') {
      *buf++ = '\0';
      return i;
    }
    *buf++ = ch;
  }
  *(--buf) = '\0';
  return i;
}
