#include "print.h"
#include "mini_uart.h"

void putc(char ch) {
  if (ch == '\r' || ch == '\n')
    uart_send_string("\r\n");
  else
    uart_send(ch);
}

int puti(int i) {
  if (i < 0 || i > 9)
    return -1;

  uart_send('0' + i);
  return 0;
}

void printi(unsigned long num) {
  if (num == 0)
    puti(0);

  int integer[64];
  int idx = 0;
  while (num) {
    integer[idx++] = num % 10;
    num /= 10;
  }
  for (int i = idx - 1; i >= 0; i--)
    puti(integer[i]);
}

void printhex(unsigned int num, bool prefix) {
  if (prefix)
    print("0x");

  int shb = 28;
  while (shb >= 0) {
    int cur = (num >> shb) % 16;

    if (cur >= 10)
      putc('A' + cur - 10);
    else
      putc('0' + cur);

    shb -= 4;
  }
}

void print(char *str) {
  while (*str) {
    putc(*str++);
  }
}

void printf(char *str, ...) {
  __builtin_va_list arg;
  __builtin_va_start(arg, str);

  while (*str) {
    if (*str == '%') {
      str++;
      if (*str == 'd')
        printi(__builtin_va_arg(arg, int));
      else if (*str == 'c')
        putc(__builtin_va_arg(arg, int)); // implicit conversion from int to char
      else if (*str == 's')
        print(__builtin_va_arg(arg, char*));
      else if (*str == '#' && *++str == 'X')
        printhex(__builtin_va_arg(arg, unsigned int), true);
      else if (*str == 'X')
        printhex(__builtin_va_arg(arg, unsigned int), false);
      else if (*str == '%')
        putc('%');
      str++;
    } else {
      putc(*str++);
    }
  }
  
  __builtin_va_end(arg);
}
