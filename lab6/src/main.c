#include "dtb.h"
#include "exception.h"
#include "heap.h"
#include "initrd.h"
#include "interrupt.h"
#include "loader.h"
#include "mem.h"
#include "syscall.h"
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
  thread_init();
  preserve(0, 0x5000000);
  preserve(0x8200000, 0x2000000); // For user program
  smalloc_init();
  fdt_find_do(dtb_location, "linux,initrd-start", initrd_fdt_callback);
  uart_puts("test_thread\n");
  core_timer_enable();
  // terminal_run();
  terminal_run_thread();
  // idle();

  return 0;
}
