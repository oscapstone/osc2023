#include "dtb.h"
#include "exception.h"
#include "heap.h"
#include "initrd.h"
#include "interrupt.h"
#include "mem.h"
#include "terminal.h"
#include "thread.h"
#include "uart.h"
#include <stdint.h>

extern void set_exception_vector_table(void);

int main(void *dtb_location) {
  uart_setup();
  heap_init();
  pmalloc_init();
  set_exception_vector_table();
  enable_int();
  preserve(0, 0x5000000);
  preserve(0x8200000, 0x16000);
  smalloc_init();
  core_timer_enable();
  fdt_find_do(dtb_location, "linux,initrd-start", initrd_fdt_callback);
  test_thread_queue();
  terminal_run();

  return 0;
}
