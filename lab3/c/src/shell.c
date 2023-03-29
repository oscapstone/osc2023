#include "oscos/shell.h"

#include <stdbool.h>
#include <stdnoreturn.h>

#include "oscos/initrd.h"
#include "oscos/libc/string.h"
#include "oscos/reset.h"
#include "oscos/serial.h"
#include "oscos/user_program.h"

#define MAX_CMD_LEN 78

static bool _shell_is_initrd_valid;

static void _shell_init(void) { _shell_is_initrd_valid = initrd_init(); }

static void _shell_print_prompt(void) { serial_fputs("# "); }

static size_t _shell_read_cmd(char *const buf, const size_t n) {
  size_t cmd_len = 0;

  for (;;) {
    char c;

    int read_result;
    while ((read_result = serial_getc_nonblock()) < 0) {
      __asm__ __volatile__("wfi");
    }
    c = read_result;

    if (c == '\n') {
      serial_putc('\n');
      break;
    } else if (c == '\x7f') { // Backspace.
      if (cmd_len > 0) {
        serial_fputs("\b \b");
        cmd_len--;
      }
    } else {
      serial_putc(c);
      if (n > 0 && cmd_len < n - 1) {
        buf[cmd_len] = c;
      }
      cmd_len++;
    }
  }

  if (n == 0) {
    cmd_len = 0;
  } else if (cmd_len >= n - 1) {
    cmd_len = n - 1;
  }

  if (n > 0) {
    buf[cmd_len] = '\0';
  }

  return cmd_len;
}

static void _shell_do_cmd_help(void) {
  serial_puts("help   : print this help menu\n"
              "hello  : print Hello World!\n"
              "reboot : reboot the device\n"
              "ls     : list all files in the initial ramdisk\n"
              "cat    : print the content of a file in the initial ramdisk\n"
              "exec   : run a user program in the initial ramdisk");
}

static void _shell_do_cmd_hello(void) { serial_puts("Hello World!"); }

noreturn static void _shell_do_cmd_reboot(void) { reboot(); }

static void _shell_do_cmd_ls(void) {
  if (!_shell_is_initrd_valid) {
    serial_puts("oscsh: ls: initrd is invalid");
    return;
  }

  INITRD_FOR_ENTRY(entry) { serial_puts(CPIO_NEWC_PATHNAME(entry)); }
}

static void _shell_do_cmd_cat(void) {
  if (!_shell_is_initrd_valid) {
    serial_puts("oscsh: cat: initrd is invalid");
    return;
  }

  serial_fputs("Filename: ");

  char filename_buf[MAX_CMD_LEN + 1];
  _shell_read_cmd(filename_buf, MAX_CMD_LEN + 1);

  const cpio_newc_entry_t *const entry =
      initrd_find_entry_by_pathname(filename_buf);
  if (!entry) {
    serial_puts("oscsh: cat: no such file or directory");
    return;
  }

  const uint32_t mode = CPIO_NEWC_HEADER_VALUE(entry, mode);
  const uint32_t file_type = mode & CPIO_NEWC_MODE_FILE_TYPE_MASK;
  if (file_type == CPIO_NEWC_MODE_FILE_TYPE_REG) {
    serial_write(CPIO_NEWC_FILE_DATA(entry), CPIO_NEWC_FILESIZE(entry));
  } else if (file_type == CPIO_NEWC_MODE_FILE_TYPE_DIR) {
    serial_puts("oscsh: cat: is a directory");
  } else if (file_type == CPIO_NEWC_MODE_FILE_TYPE_LNK) {
    serial_fputs("Symbolic link to: ");
    serial_write(CPIO_NEWC_FILE_DATA(entry), CPIO_NEWC_FILESIZE(entry));
    serial_putc('\n');
  } else {
    serial_puts("oscsh: cat: unknown file type");
  }
}

static void _shell_do_cmd_exec(void) {
  if (!_shell_is_initrd_valid) {
    serial_puts("oscsh: exec: initrd is invalid");
    return;
  }

  serial_fputs("Filename: ");

  char filename_buf[MAX_CMD_LEN + 1];
  _shell_read_cmd(filename_buf, MAX_CMD_LEN + 1);

  const cpio_newc_entry_t *const entry =
      initrd_find_entry_by_pathname(filename_buf);
  if (!entry) {
    serial_puts("oscsh: exec: no such file or directory");
    return;
  }

  const void *user_program_start;
  size_t user_program_len;

  const uint32_t mode = CPIO_NEWC_HEADER_VALUE(entry, mode);
  const uint32_t file_type = mode & CPIO_NEWC_MODE_FILE_TYPE_MASK;
  if (file_type == CPIO_NEWC_MODE_FILE_TYPE_REG) {
    user_program_start = CPIO_NEWC_FILE_DATA(entry);
    user_program_len = CPIO_NEWC_FILESIZE(entry);
  } else if (file_type == CPIO_NEWC_MODE_FILE_TYPE_DIR) {
    serial_puts("oscsh: exec: is a directory");
    return;
  } else if (file_type == CPIO_NEWC_MODE_FILE_TYPE_LNK) {
    serial_fputs("oscos: exec: is a symbolic link to: ");
    serial_write(CPIO_NEWC_FILE_DATA(entry), CPIO_NEWC_FILESIZE(entry));
    serial_putc('\n');
    return;
  } else {
    serial_puts("oscsh: exec: unknown file type");
    return;
  }

  if (!load_user_program(user_program_start, user_program_len)) {
    serial_puts("oscsh: exec: user program too long");
    return;
  }

  run_user_program();
}

static void _shell_cmd_not_found(const char *const cmd) {
  serial_fputs("oscsh: ");
  serial_fputs(cmd);
  serial_puts(": command not found");
}

void run_shell(void) {
  _shell_init();

  for (;;) {
    _shell_print_prompt();

    char cmd_buf[MAX_CMD_LEN + 1];
    const size_t cmd_len = _shell_read_cmd(cmd_buf, MAX_CMD_LEN + 1);

    if (cmd_len == 0) {
      // No-op.
    } else if (strcmp(cmd_buf, "help") == 0) {
      _shell_do_cmd_help();
    } else if (strcmp(cmd_buf, "hello") == 0) {
      _shell_do_cmd_hello();
    } else if (strcmp(cmd_buf, "reboot") == 0) {
      _shell_do_cmd_reboot();
    } else if (strcmp(cmd_buf, "ls") == 0) {
      _shell_do_cmd_ls();
    } else if (strcmp(cmd_buf, "cat") == 0) {
      _shell_do_cmd_cat();
    } else if (strcmp(cmd_buf, "exec") == 0) {
      _shell_do_cmd_exec();
    } else {
      _shell_cmd_not_found(cmd_buf);
    }
  }
}
