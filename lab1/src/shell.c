#include "shell.h"
#include "string.h"

#define BUF_SIZE 32

void print_hello() {
  print("Hello World!\n");
}

void print_help() {
  print("help\t: print this help menu\n");
  print("hello\t: print Hello World!\n");
  print("reboot\t: reboot the device\n");
}

void print_unsupport() {
  print("Command not found.\n");
}

void shell() {
  char buf[BUF_SIZE];

  print("Welcome to rpi-os shell!\n");
  print("Enter \"help\" for a list of built-in commands.\n");

  while (1) {
    print("# ");
    if (read(buf, BUF_SIZE) != 0)
      continue;
    if (streq(buf, "help") == 0) {
      print_help();
    }
    else if (streq(buf, "hello") == 0) {
      print_hello();
    }
    else if (streq(buf, "reboot") == 0) {
      // FIXME
      print_hello();
    }
    else {
      print_unsupport();
    }
  }
}
