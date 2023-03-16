#include "mini_uart.h"
#include "devtree.h"
#include "shell.h"
#include "print.h"
#include "string.h"

void initramfs_callback(const char *nodename, const char *propname, void *value) {
  if (streq(nodename, "chosen") == 0 &&
      streq(propname, "linux,initrd-start") == 0) {
    printf("%s\t%s\n", nodename, propname);
    printf("%#X\n", value);
  }
  return;
}

void kernel_main() {
  uart_init();
  fdt_traverse(initramfs_callback);
  shell();
}
