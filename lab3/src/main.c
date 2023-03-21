#include "dtb.h"
#include "exception.h"
#include "heap.h"
#include "initrd.h"
#include "terminal.h"
#include "uart.h"
#include <stdint.h>

extern void set_exception_vector_table(void);

int main(void *dtb_location) {
  uart_setup();
  heap_init();
  set_exception_vector_table();
  fdt_find_do(dtb_location, "linux,initrd-start", initrd_fdt_callback);
  terminal_run();

  return 0;
}
