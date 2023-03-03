#include "shell.h"
#include "string.h"
#include "print.h"
#include "mini_uart.h"
#include "mailbox.h"
#include "reboot.h"

#define BUF_SIZE 32

int read_cmd(char *buf, int len) {
  char ch;
  // char prev[BUF_SIZE];
  // strcpy(prev, buf);
  // char *prvptr = prev;
  // char *bufsave = buf;

  while (len--) {
    ch = uart_recv();
    // if (ch == '\033') {
    //   ch = uart_recv();
    //   ch = uart_recv();
    //   if (ch == 65) {
    //     while (*prvptr) {
    //       buf = &bufsave;
    //       *buf = *prvptr++;
    //       putc(*buf++);
    //       len--;
    //     }
    //   }
    //   continue;
    // }
  putc(ch);
  if (ch == '\n' || ch == '\r') {
      *buf++ = '\0';
      return 0;
    }
    *buf++ = ch;
  }
  *(--buf) = '\0';
  print("\nBuffer is already full!\n");
  return -1;
}

void print_hello() {
  print("Hello World!\n");
}

void print_help() {
  print("help\t: print this help menu\n");
  print("hello\t: print Hello World!\n");
  print("reboot\t: reboot the device\n");
  print("info\t: print hardware info\n");
}

void print_unsupport(char *buf) {
  print("Command not found: ");
  print(buf);
  print("\n");
}

void shell() {
  char buf[BUF_SIZE];

  print("Welcome to rpi-os shell!\n");
  print("Enter \"help\" for a list of built-in commands.\n");

  while (1) {
    print("# ");
    if (read_cmd(buf, BUF_SIZE) != 0 || *buf == '\0')
      continue;
    if (streq(buf, "help") == 0) {
      print_help();
    }
    else if (streq(buf, "hello") == 0) {
      print_hello();
    }
    else if (streq(buf, "reboot") == 0) {
      reset(100);
    }
    else if (streq(buf, "info") == 0) {
      get_board_revision();
      get_arm_memory();
    }
    else {
      print_unsupport(buf);
    }
  }
}
