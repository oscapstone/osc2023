#include "mini_uart.h"
#include "devtree.h"
#include "cpio.h"
#include "shell.h"

void kernel_main() {
  uart_init();
  fdt_traverse(initramfs_callback);
  shell();
}
