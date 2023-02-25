#include "mini_uart.h"
#include "shell.h"

void kernel_main() {
  uart_init();

  shell();
}
