#include "shell.h"
#include "string.h"
#include "print.h"
#include "read.h"
#include "mini_uart.h"
#include "mailbox.h"
#include "reboot.h"
#include "cpio.h"

#define BUF_SIZE 32

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
  printf("Command not found: %s\n", buf);
}

void shell() {
  char buf[BUF_SIZE];
  bool interact = true;

  print("Welcome to rpi-os shell!\n");
  print("Enter \"help\" for a list of built-in commands.\n");

  while (1) {
    print("# ");
    if (readline(buf, BUF_SIZE, interact) <= 0 || *buf == '\0')
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
    else if (streq(buf, "ls") == 0) {
      cpio_ls();
    }
    else if (strstartwith(buf, "cat ") == 0) {
      char *files = buf + 4;
      char *start = files;
      printf("%s\n", files);
      while (*files) {
        if (*files == ' ') {
          cpio_cat(start, files - start);
          start = ++files;
        } else {
          ++files;
        }
      }
      cpio_cat(start, files - start);
    }
    else {
      print_unsupport(buf);
    }
  }
}
