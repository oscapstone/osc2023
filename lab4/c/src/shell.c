#include "oscos/shell.h"

#include <stdnoreturn.h>

#include "oscos/console.h"
#include "oscos/drivers/mailbox.h"
#include "oscos/drivers/pm.h"
#include "oscos/initrd.h"
#include "oscos/libc/ctype.h"
#include "oscos/libc/inttypes.h"
#include "oscos/libc/string.h"
#include "oscos/mem/simple-alloc.h"
#include "oscos/timer/timeout.h"
#include "oscos/user-program.h"
#include "oscos/utils/time.h"

#define MAX_CMD_LEN 78

static void _shell_print_prompt(void) { console_fputs("# "); }

static size_t _shell_read_line(char *const buf, const size_t n) {
  size_t cmd_len = 0;

  for (;;) {
    const char c = console_getc();

    if (c == '\n') {
      console_putc('\n');
      break;
    } else if (c == '\x7f') { // Backspace.
      if (cmd_len > 0) {
        console_fputs("\b \b");
        cmd_len--;
      }
    } else {
      console_putc(c);
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
  console_puts(
      "help       : print this help menu\n"
      "hello      : print Hello World!\n"
      "hwinfo     : get the hardware's information by mailbox\n"
      "reboot     : reboot the device\n"
      "ls         : list all files in the initial ramdisk\n"
      "cat        : print the content of a file in the initial ramdisk\n"
      "exec       : run a user program in the initial ramdisk\n"
      "setTimeout : print a message after a timeout");
}

static void _shell_do_cmd_hello(void) { console_puts("Hello World!"); }

static void _shell_do_cmd_hwinfo(void) {
  const uint32_t board_revision = mailbox_get_board_revision();
  const arm_memory_t arm_memory = mailbox_get_arm_memory();

  console_printf("Board revision: 0x%" PRIx32 "\nARM memory: Base: 0x%" PRIx32
                 ", Size: 0x%" PRIx32 "\n",
                 board_revision, arm_memory.base, arm_memory.size);
}

noreturn static void _shell_do_cmd_reboot(void) { pm_reboot(); }

static void _shell_do_cmd_ls(void) {
  if (!initrd_is_init()) {
    console_puts("oscsh: ls: initrd is invalid");
    return;
  }

  INITRD_FOR_ENTRY(entry) { console_puts(CPIO_NEWC_PATHNAME(entry)); }
}

static void _shell_do_cmd_cat(void) {
  if (!initrd_is_init()) {
    console_puts("oscsh: cat: initrd is invalid");
    return;
  }

  console_fputs("Filename: ");

  char filename_buf[MAX_CMD_LEN + 1];
  _shell_read_line(filename_buf, MAX_CMD_LEN + 1);

  const cpio_newc_entry_t *const entry =
      initrd_find_entry_by_pathname(filename_buf);
  if (!entry) {
    console_puts("oscsh: cat: no such file or directory");
    return;
  }

  const uint32_t mode = CPIO_NEWC_HEADER_VALUE(entry, mode);
  const uint32_t file_type = mode & CPIO_NEWC_MODE_FILE_TYPE_MASK;
  if (file_type == CPIO_NEWC_MODE_FILE_TYPE_REG) {
    console_write(CPIO_NEWC_FILE_DATA(entry), CPIO_NEWC_FILESIZE(entry));
  } else if (file_type == CPIO_NEWC_MODE_FILE_TYPE_DIR) {
    console_puts("oscsh: cat: is a directory");
  } else if (file_type == CPIO_NEWC_MODE_FILE_TYPE_LNK) {
    console_fputs("Symbolic link to: ");
    console_write(CPIO_NEWC_FILE_DATA(entry), CPIO_NEWC_FILESIZE(entry));
    console_putc('\n');
  } else {
    console_puts("oscsh: cat: unknown file type");
  }
}

static void _shell_do_cmd_exec(void) {
  if (!initrd_is_init()) {
    console_puts("oscsh: exec: initrd is invalid");
    return;
  }

  console_fputs("Filename: ");

  char filename_buf[MAX_CMD_LEN + 1];
  _shell_read_line(filename_buf, MAX_CMD_LEN + 1);

  const cpio_newc_entry_t *const entry =
      initrd_find_entry_by_pathname(filename_buf);
  if (!entry) {
    console_puts("oscsh: exec: no such file or directory");
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
    console_puts("oscsh: exec: is a directory");
    return;
  } else if (file_type == CPIO_NEWC_MODE_FILE_TYPE_LNK) {
    console_fputs("oscos: exec: is a symbolic link to: ");
    console_write(CPIO_NEWC_FILE_DATA(entry), CPIO_NEWC_FILESIZE(entry));
    console_putc('\n');
    return;
  } else {
    console_puts("oscsh: exec: unknown file type");
    return;
  }

  if (!load_user_program(user_program_start, user_program_len)) {
    console_puts("oscsh: exec: user program too long");
    return;
  }

  run_user_program();
}

static void _shell_do_cmd_set_timeout(const char *const args) {
  const char *c = args;

  // Skip initial whitespaces.
  for (; *c == ' '; c++)
    ;

  // Message.

  if (!*c)
    goto invalid;

  const char *const message_start = c;
  size_t message_len = 0;

  for (; *c && *c != ' '; c++) {
    message_len++;
  }

  // Skip whitespaces.
  for (; *c == ' '; c++)
    ;

  // Seconds.

  if (!*c)
    goto invalid;

  size_t seconds = 0;
  for (; *c && *c != ' '; c++) {
    if (!isdigit(*c))
      goto invalid;
    seconds = seconds * 10 + (*c - '0');
  }

  // Skip whitespaces.
  for (; *c == ' '; c++)
    ;

  // There should be no more arguments.
  if (*c)
    goto invalid;

  // Copy the string.

  char *const message_copy = simple_alloc(message_len + 1);
  memcpy(message_copy, message_start, message_len);
  message_copy[message_len] = '\0';

  // Register the callback.
  timeout_add_timer((void (*)(void *))console_puts, message_copy,
                    seconds * NS_PER_SEC);
  return;

invalid:
  console_puts("oscsh: setTimeout: invalid command format");
}

static void _shell_cmd_not_found(const char *const cmd) {
  console_printf("oscsh: %s: command not found\n", cmd);
}

void run_shell(void) {
  for (;;) {
    _shell_print_prompt();

    char cmd_buf[MAX_CMD_LEN + 1];
    const size_t cmd_len = _shell_read_line(cmd_buf, MAX_CMD_LEN + 1);

    if (cmd_len == 0) {
      // No-op.
    } else if (strcmp(cmd_buf, "help") == 0) {
      _shell_do_cmd_help();
    } else if (strcmp(cmd_buf, "hello") == 0) {
      _shell_do_cmd_hello();
    } else if (strcmp(cmd_buf, "hwinfo") == 0) {
      _shell_do_cmd_hwinfo();
    } else if (strcmp(cmd_buf, "reboot") == 0) {
      _shell_do_cmd_reboot();
    } else if (strcmp(cmd_buf, "ls") == 0) {
      _shell_do_cmd_ls();
    } else if (strcmp(cmd_buf, "cat") == 0) {
      _shell_do_cmd_cat();
    } else if (strcmp(cmd_buf, "exec") == 0) {
      _shell_do_cmd_exec();
    } else if (strncmp(cmd_buf, "setTimeout", 10) == 0 &&
               (!cmd_buf[10] || cmd_buf[10] == ' ')) {
      _shell_do_cmd_set_timeout(cmd_buf + 10);
    } else {
      _shell_cmd_not_found(cmd_buf);
    }
  }
}
