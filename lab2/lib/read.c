#include "read.h"
#include "type.h"
#include "print.h"
#include "mini_uart.h"

int readline(char *buf, int len, bool interact) {
  char ch;
  int i = 0;
  char *ptr = buf;

  for (i = 0; i < len - 1; i++) {
    ch = uart_recv();
    if (interact == true)
      putc(ch);
    if (ch == '\n' || ch == '\r') {
      break;
    } else if (ch < 31 || ch > 128) {
      --i;
      continue;
    }
    *ptr++ = ch;
  }
  *ptr = '\0';
  return i;
}
