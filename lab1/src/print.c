#include "print.h"
#include "mini_uart.h"

void putc(char ch) {
  if (ch == '\r' || ch == '\n')
    uart_send_string("\r\n");
  else
    uart_send(ch);
  // if (ch == '\n')
  //   uart_send('\r');
  // uart_send(ch);
}

void printhex(unsigned int num) {
  print("0x");

  int shb = 28;
  while (shb >= 0) {
    int cur = (num >> shb) % 16;

    if (cur >= 10)  putc('A' + cur - 10);
    else  putc('0' + cur);

    shb -= 4;
  }
}

void print(char *str) {
  while (*str) {
    putc(*str++);
  }
}
