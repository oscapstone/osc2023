#include "oscos/shell.h"

#include <stdnoreturn.h>

#include "oscos/console.h"
#include "oscos/drivers/mailbox.h"
#include "oscos/drivers/pm.h"
#include "oscos/fs/vfs.h"
#include "oscos/initrd.h"
#include "oscos/libc/ctype.h"
#include "oscos/libc/inttypes.h"
#include "oscos/libc/string.h"
#include "oscos/mem/malloc.h"
#include "oscos/mem/page-alloc.h"
#include "oscos/sched.h"
#include "oscos/timer/timeout.h"
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

  if (n > 0) {
    buf[cmd_len > n - 1 ? n - 1 : cmd_len] = '\0';
  }

  return cmd_len;
}

static void _shell_do_cmd_help(void) {
  console_puts(
      "help        : print this help menu\n"
      "hello       : print Hello World!\n"
      "hwinfo      : get the hardware's information by mailbox\n"
      "reboot      : reboot the device\n"
      "ls          : list all files in the initial ramdisk\n"
      "cat         : print the content of a file in the initial ramdisk\n"
      "exec        : run a user program in the initial ramdisk\n"
      "setTimeout  : print a message after a timeout\n"
      "alloc-pages : allocates a block of page frames using the page frame "
      "allocator\n"
      "free-pages  : frees a block of page frames allocated using the page "
      "frame allocator");
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

  if (!process_create()) {
    console_puts("oscsh: exec: out of memory");
    return;
  }

  exec_first(user_program_start, user_program_len);

  // If execution reaches here, then exec failed.
  console_puts("oscsh: exec: out of memory");
}

static void _shell_cmd_set_timeout_timer_callback(char *const message) {
  console_puts(message);
  free(message);
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

  char *const message_copy = malloc(message_len + 1);
  if (!message_copy) {
    console_puts("oscsh: setTimeout: out of memory");
    return;
  }
  memcpy(message_copy, message_start, message_len);
  message_copy[message_len] = '\0';

  // Register the callback.
  timeout_add_timer_ns((void (*)(void *))_shell_cmd_set_timeout_timer_callback,
                       message_copy, seconds * NS_PER_SEC);
  return;

invalid:
  console_puts("oscsh: setTimeout: invalid command format");
}

static void _shell_do_cmd_alloc_pages(void) {
  console_fputs("Order (decimal, leave blank for 0): ");

  char digit_buf[3];
  const size_t digit_len = _shell_read_line(digit_buf, 3);
  if (digit_len > 2)
    goto invalid;

  size_t order = 0;
  for (const char *c = digit_buf; *c; c++) {
    if (!isdigit(*c))
      goto invalid;
    order = order * 10 + (*c - '0');
  }

  const spage_id_t page = alloc_pages(order);
  if (page < 0) {
    console_puts("oscsh: alloc-pages: out of memory");
    return;
  }

  console_printf("Page ID: 0x%" PRIxPAGEID "\n", (page_id_t)page);
  return;

invalid:
  console_puts("oscsh: alloc-pages: invalid order");
}

static void _shell_do_cmd_free_pages(void) {
  console_fputs(
      "Page number of the first page (lowercase hexadecimal without prefix): ");

  char digit_buf[9];
  const size_t digit_len = _shell_read_line(digit_buf, 9);
  if (digit_len > 8)
    goto invalid;

  page_id_t page_id = 0;
  for (const char *c = digit_buf; *c; c++) {
    page_id_t digit_value;
    if (isdigit(*c)) {
      digit_value = *c - '0';
    } else if ('a' <= *c && *c <= 'f') {
      digit_value = *c - 'a' + 10;
    } else {
      goto invalid;
    }
    page_id = page_id << 4 | digit_value;
  }

  free_pages(page_id);
  return;

invalid:
  console_puts("oscsh: free-pages: invalid page number");
}

static void _shell_do_cmd_vfs_test_1(void) {
  char buf[11] = {0};
  struct file *foo, *bar;
  int result;

  result = vfs_open("/foo", O_CREAT, &foo);
  console_printf("open(\"/foo\", O_CREAT) = %d\n", result);
  if (result < 0)
    return;

  result = vfs_write(foo, "aaaaa", 5);
  console_printf("write(\"/foo\", \"aaaaa\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_close(foo);
  console_printf("close(\"/foo\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_open("/bar", O_CREAT, &bar);
  console_printf("open(\"/bar\", O_CREAT) = %d\n", result);
  if (result < 0)
    return;

  result = vfs_write(bar, "bbbbb", 5);
  console_printf("write(\"/bar\", \"bbbbb\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_close(bar);
  console_printf("close(\"/bar\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_open("/foo", O_CREAT, &foo);
  console_printf("open(\"/foo\", O_CREAT) = %d\n", result);
  if (result < 0)
    return;

  result = vfs_open("/bar", O_CREAT, &bar);
  console_printf("open(\"/bar\", O_CREAT) = %d\n", result);
  if (result < 0)
    return;

  result = vfs_read(foo, buf, 10);
  console_printf("read(\"/foo\", ...) = %d\n", result);
  if (result < 0)
    return;
  console_puts(buf);

  memset(buf, 0, 10);
  result = vfs_read(bar, buf, 10);
  console_printf("read(\"/bar\", ...) = %d\n", result);
  if (result < 0)
    return;
  console_puts(buf);

  result = vfs_close(foo);
  console_printf("close(\"/foo\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_close(bar);
  console_printf("close(\"/bar\") = %d\n", result);
  if (result < 0)
    return;
}

static void _shell_do_cmd_vfs_test_2(void) {
  char buf[11] = {0};
  struct file *foo, *bar;
  int result;

  result = vfs_mkdir("/dir");
  console_printf("mkdir(\"/dir\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_open("/dir/foo", O_CREAT, &foo);
  console_printf("open(\"/dir/foo\", O_CREAT) = %d\n", result);
  if (result < 0)
    return;

  result = vfs_write(foo, "aaaaa", 5);
  console_printf("write(\"/dir/foo\", \"aaaaa\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_close(foo);
  console_printf("close(\"/dir/foo\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_open("/dir/bar", O_CREAT, &bar);
  console_printf("open(\"/dir/bar\", O_CREAT) = %d\n", result);
  if (result < 0)
    return;

  result = vfs_write(bar, "bbbbb", 5);
  console_printf("write(\"/dir/bar\", \"bbbbb\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_close(bar);
  console_printf("close(\"/dir/bar\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_open("/dir/foo", O_CREAT, &foo);
  console_printf("open(\"/dir/foo\", O_CREAT) = %d\n", result);
  if (result < 0)
    return;

  result = vfs_open("/dir/bar", O_CREAT, &bar);
  console_printf("open(\"/dir/bar\", O_CREAT) = %d\n", result);
  if (result < 0)
    return;

  result = vfs_read(foo, buf, 10);
  console_printf("read(\"/dir/foo\", ...) = %d\n", result);
  if (result < 0)
    return;
  console_puts(buf);

  memset(buf, 0, 10);
  result = vfs_read(bar, buf, 10);
  console_printf("read(\"/dir/bar\", ...) = %d\n", result);
  if (result < 0)
    return;
  console_puts(buf);

  result = vfs_close(foo);
  console_printf("close(\"/dir/foo\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_close(bar);
  console_printf("close(\"/dir/bar\") = %d\n", result);
  if (result < 0)
    return;
}

static void _shell_do_cmd_vfs_test_3(void) {
  char buf[11] = {0};
  struct file *foo, *bar;
  int result;

  result = vfs_mkdir("/dir");
  console_printf("mkdir(\"/dir\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_mount("/dir", "tmpfs");
  console_printf("mount(\"/dir\", \"tmpfs\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_open("/dir/foo", O_CREAT, &foo);
  console_printf("open(\"/dir/foo\", O_CREAT) = %d\n", result);
  if (result < 0)
    return;

  result = vfs_write(foo, "aaaaa", 5);
  console_printf("write(\"/dir/foo\", \"aaaaa\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_close(foo);
  console_printf("close(\"/dir/foo\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_open("/dir/bar", O_CREAT, &bar);
  console_printf("open(\"/dir/bar\", O_CREAT) = %d\n", result);
  if (result < 0)
    return;

  result = vfs_write(bar, "bbbbb", 5);
  console_printf("write(\"/dir/bar\", \"bbbbb\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_close(bar);
  console_printf("close(\"/dir/bar\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_open("/dir/foo", O_CREAT, &foo);
  console_printf("open(\"/dir/foo\", O_CREAT) = %d\n", result);
  if (result < 0)
    return;

  result = vfs_open("/dir/bar", O_CREAT, &bar);
  console_printf("open(\"/dir/bar\", O_CREAT) = %d\n", result);
  if (result < 0)
    return;

  result = vfs_read(foo, buf, 10);
  console_printf("read(\"/dir/foo\", ...) = %d\n", result);
  if (result < 0)
    return;
  console_puts(buf);

  memset(buf, 0, 10);
  result = vfs_read(bar, buf, 10);
  console_printf("read(\"/dir/bar\", ...) = %d\n", result);
  if (result < 0)
    return;
  console_puts(buf);

  result = vfs_close(foo);
  console_printf("close(\"/dir/foo\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_close(bar);
  console_printf("close(\"/dir/bar\") = %d\n", result);
  if (result < 0)
    return;
}

static void _shell_do_cmd_vfs_test_4(void) {
  char buf[11] = {0};
  struct file *foo, *bar;
  int result;

  result = vfs_mkdir("/dir");
  console_printf("mkdir(\"/dir\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_mount("/dir", "tmpfs");
  console_printf("mount(\"/dir\", \"tmpfs\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_mkdir("/dir/dir");
  console_printf("mkdir(\"/dir/dir\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_open("/dir/foo", O_CREAT, &foo);
  console_printf("open(\"/dir/foo\", O_CREAT) = %d\n", result);
  if (result < 0)
    return;

  result = vfs_write(foo, "aaaaa", 5);
  console_printf("write(\"/dir/foo\", \"aaaaa\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_close(foo);
  console_printf("close(\"/dir/foo\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_open("/dir/bar", O_CREAT, &bar);
  console_printf("open(\"/dir/bar\", O_CREAT) = %d\n", result);
  if (result < 0)
    return;

  result = vfs_write(bar, "bbbbb", 5);
  console_printf("write(\"/dir/bar\", \"bbbbb\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_close(bar);
  console_printf("close(\"/dir/bar\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_open("/dir/./dir/../../dir/./foo", O_CREAT, &foo);
  console_printf("open(\"/dir/./dir/../../dir/./foo\", O_CREAT) = %d\n",
                 result);
  if (result < 0)
    return;

  result = vfs_open("/dir/./dir/../../dir/./bar", O_CREAT, &bar);
  console_printf("open(\"/dir/./dir/../../dir/./bar\", O_CREAT) = %d\n",
                 result);
  if (result < 0)
    return;

  result = vfs_read(foo, buf, 10);
  console_printf("read(\"/dir/./dir/../../dir/./foo\", ...) = %d\n", result);
  if (result < 0)
    return;
  console_puts(buf);

  memset(buf, 0, 10);
  result = vfs_read(bar, buf, 10);
  console_printf("read(\"/dir/./dir/../../dir/./bar\", ...) = %d\n", result);
  if (result < 0)
    return;
  console_puts(buf);

  result = vfs_close(foo);
  console_printf("close(\"/dir/./dir/../../dir/./foo\") = %d\n", result);
  if (result < 0)
    return;

  result = vfs_close(bar);
  console_printf("close(\"/dir/./dir/../../dir/./bar\") = %d\n", result);
  if (result < 0)
    return;
}

static void _shell_do_cmd_vfs_test_5(void) {
  unsigned char buf[8] = {0};
  struct file *vfs1;
  int result;

  result = vfs_open("/initramfs/vfs1.img", 0, &vfs1);
  console_printf("open(\"/initramfs/vfs1.img\", 0) = %d\n", result);
  if (result < 0)
    return;

  result = vfs_read(vfs1, buf, 8);
  console_printf("read(\"/initramfs/vfs1.img\", ...) = %d\n", result);
  if (result < 0)
    return;
  for (size_t i = 0; i < 8; i++) {
    console_printf("\\x%02hhx", buf[i]);
  }
  console_putc('\n');

  result = vfs_close(vfs1);
  console_printf("close(\"/initramfs/vfs1.img\") = %d\n", result);
  if (result < 0)
    return;
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
    } else if (strcmp(cmd_buf, "alloc-pages") == 0) {
      _shell_do_cmd_alloc_pages();
    } else if (strcmp(cmd_buf, "free-pages") == 0) {
      _shell_do_cmd_free_pages();
    } else if (strcmp(cmd_buf, "vfs-test-1") == 0) {
      _shell_do_cmd_vfs_test_1();
    } else if (strcmp(cmd_buf, "vfs-test-2") == 0) {
      _shell_do_cmd_vfs_test_2();
    } else if (strcmp(cmd_buf, "vfs-test-3") == 0) {
      _shell_do_cmd_vfs_test_3();
    } else if (strcmp(cmd_buf, "vfs-test-4") == 0) {
      _shell_do_cmd_vfs_test_4();
    } else if (strcmp(cmd_buf, "vfs-test-5") == 0) {
      _shell_do_cmd_vfs_test_5();
    } else {
      _shell_cmd_not_found(cmd_buf);
    }
  }
}
