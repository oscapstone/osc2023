#include "shell.h"

void shell_input(char* cmd) {
  int idx = 0;
  while (1) {
    uart_printf("\r# %s", cmd);

    char s = uart_getchar();
    if (s == '\n') {
      uart_printf("\n");
      return;
    }

    // only allow printable characters
    if (20 <= s && s < 127) {
      cmd[idx] = s;
      cmd[idx + 1] = '\0';
      idx++;
    }
  }
}

void shell_exec(const char* cmd) {
  if (*cmd == '\0') {
    return;
  }

  if (!my_strcmp(cmd, "help")) {
    uart_puts(
        "help\tprint this help menu\n"
        "hello\tprint \"Hello, world!\"\n"
        "info\tprint board revision & arm memory\n"
        "reboot\treboot the device");
    return;
  }

  if (!my_strcmp(cmd, "hello")) {
    uart_puts("Hello, world!");
    return;
  }

  if (!my_strcmp(cmd, "info")) {
    uint32_t board_rev;
    uint32_t mem_base;
    uint32_t mem_size;

    mbox_get_board_revision(&board_rev);
    mbox_get_arm_memory(&mem_base, &mem_size);

    uart_printf(
        "Board revision: 0x%X\n"
        "Memory base: 0x%X\n"
        "Memory size: 0x%X\n",
        board_rev, mem_base, mem_size);
    return;
  }

  if (!my_strcmp(cmd, "reboot")) {
    uart_puts("Rebooting...");
    wdt_restart();
    return;
  }

  uart_printf("command not found: %s\n", cmd);
}

void shell() {
  uart_init();

  char cmd[COMMAND_SIZE];
  cmd[0] = '\0';

  // clang-format off
  uart_printf(
    "                                                                                       \n"
    "   ____  _____ ____  _______   ____ ___  _____             _____________  __________ __\n"
    "  / __ \\/ ___// __ \\/  _/__ \\ / __ \\__ \\|__  /            / ____/ ___/\\ \\/ / ____/ // /\n"
    " / / / /\\__ \\/ / / // / __/ // / / /_/ / /_ <   ______   / /    \\__ \\  \\  /___ \\/ // /_\n"
    "/ /_/ /___/ / /_/ // / / __// /_/ / __/___/ /  /_____/  / /___ ___/ /  / /___/ /__  __/\n"
    "\\____//____/_____/___//____/\\____/____/____/            \\____//____/  /_/_____/  /_/   \n"
    "                                                                                       \n"
  );
  // clang-format on

  while (1) {
    shell_input(cmd);
    shell_exec(cmd);
    uart_printf("\n");
    cmd[0] = '\0';
  }
}
