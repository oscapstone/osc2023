#include "io.h"
#include "uart.h"



void print_char(const char c) {
  if (c == '\n') mini_uart_send('\r');
  mini_uart_send(c);
}

void print_string(const char *str) {
  while (*str) {
    print_char(*str++);
  }
}


void print_h(unsigned int num) {
  print_string("0x");
  int h = 28;
  while (h >= 0) {
    char ch = (num >> h) & 0xF;
    if (ch >= 10) ch += 'A' - 10;
    else ch += '0';
    print_char(ch);
    h -= 4;
  }
}



void print_num(int num) {
  if (num == 0) {
    print_char('0');
    return;
  }
  if (num < 0) {
    print_char('-');
    num = -num;
  }
  char buf[10];
  int len = 0;
  while (num > 0) {
    buf[len++] = (char)(num%10)+'0';
    num /= 10;
  }
  for (int i = len-1; i >= 0; i--) {
    print_char(buf[i]);
  }
}
