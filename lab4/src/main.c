#include "dtb.h"
#include "exception.h"
#include "heap.h"
#include "initrd.h"
#include "interrupt.h"
#include "mem.h"
#include "terminal.h"
#include "uart.h"
#include <stdint.h>

extern void set_exception_vector_table(void);

int main(void *dtb_location) {
  uart_setup();
  heap_init();
  pmalloc_init();
  preserve(0, 0x10000000);
  void* a = pmalloc(4);
  void* b = pmalloc(4);
  void* c = pmalloc(4);
  void* d = pmalloc(0);
  void* e = pmalloc(0);
  void* f = pmalloc(0);
  pfree(d);
  pfree(e);
  pfree(f);
  pfree(c);
  pfree(b);
  pfree(a);
  set_exception_vector_table();
  //core_timer_enable();
  fdt_find_do(dtb_location, "linux,initrd-start", initrd_fdt_callback);
  //enable_int();
  terminal_run();

  return 0;
}
