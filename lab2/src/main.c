#include "terminal.h"
#include "uart.h"
#include "heap.h"
#include "dtb.h"
#include "initrd.h"
#include <stdint.h>

int main(void* dtb_location) {
  uart_setup();
  heap_init();
  uart_puth(dtb_location);
  //fdt_dump(dtb_location);
  uart_puth(dtb_location);
  uart_puts("initrams(before): ");
  uart_puth(initrd_getLo());
  uart_puts("\n");
  fdt_find_do(dtb_location, "linux,initrd-start", initrd_fdt_callback);
  uart_puts("initrams(after): ");
  uart_puth(initrd_getLo());
  uart_puts("\n");
  terminal_run();

  return 0;
}
