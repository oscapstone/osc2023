#include "oscos/shell.h"

#include "oscos/libc/string.h"
#include "oscos/serial.h"

#define MAX_CMD_LEN 78

static void _shell_print_prompt(void) { serial_fputs("# "); }

static size_t _shell_read_cmd(char *const buf, const size_t n) {
  size_t cmd_len = 0;

  for (;;) {
    const char c = serial_getc();
    serial_putc(c);

    if (c == '\n')
      break;

    if (cmd_len < n - 1) {
      buf[cmd_len++] = c;
    }
  }

  buf[cmd_len] = '\0';
  return cmd_len;
}

static void _shell_do_cmd_help(void) {
  serial_lock();
  serial_puts("help  : print this help menu\n"
              "hello : print Hello World!");
  serial_unlock();
}

static void _shell_do_cmd_hello(void) {
  serial_lock();
  serial_puts("Hello World!");
  serial_unlock();
}

static void _shell_cmd_not_found(const char *const cmd) {
  serial_lock();

  serial_fputs("oscsh: ");
  serial_fputs(cmd);
  serial_puts(": command not found");

  serial_unlock();
}

void run_shell(void) {
  for (;;) {
    serial_lock();

    _shell_print_prompt();

    char cmd_buf[MAX_CMD_LEN + 1];
    _shell_read_cmd(cmd_buf, MAX_CMD_LEN + 1);

    serial_unlock();

    if (strcmp(cmd_buf, "help") == 0) {
      _shell_do_cmd_help();
    } else if (strcmp(cmd_buf, "hello") == 0) {
      _shell_do_cmd_hello();
    } else {
      _shell_cmd_not_found(cmd_buf);
    }
  }
}
