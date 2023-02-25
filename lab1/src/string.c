#include "string.h"
#include "mini_uart.h"

void putc(char ch) {
  if (ch == '\r' || ch == '\n')
    uart_send_string("\r\n");
  else
    uart_send(ch);
}

void print(char *str) {
  while (*str) {
    putc(*str++);
  }
}

int streq(const char *str1, const char *str2) {
  while (*str1 != '\0' && *str2 != '\0') {
    if (*str1++ != *str2++)
      return -1;
  }
  if (*str1 != *str2)
      return -1;
  return 0;
}

int read(char *buf, int len) {
  char ch;
  while (len--) {
    ch = uart_recv();
    putc(ch);
    if (ch == '\r') {
      *buf++ = '\0';
      return 0;
    }
    *buf++ = ch;
  }
  *(--buf) = '\0';
  print("\nBuffer is already full!\n");
  return -1;
}
