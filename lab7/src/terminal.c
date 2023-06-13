#include "terminal.h"
#include "command.h"
#include "mailbox.h"
#include "str.h"
#include "uart.h"

//#define BUF_LEN 256

void terminal_run() {
  char input_buf[BUF_LEN + 1];
  uart_puts("=======Terminal start!!======\n");

  while (1) {
    uart_putc('>');

    uart_gets(input_buf);

    command_exec(input_buf);
  }
}

// This implementation is slow but as for now the
// number of commands is small.
int command_exec(const char *in) {
  int i = 0;
  while (1) {
    // If the cannot find the command, just return.
    if (!strcmp(commands[i].name, "NULL")) {
      invalid_command(in);
      return 1;
    }
    // Target command found
    if (!strcmp(in, commands[i].name)) {
      commands[i].func();
      return 0;
    }
    i++;
  }
  return 0;
}
