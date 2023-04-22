#include "dtb.h"
#include "exception.h"
#include "heap.h"
#include "initrd.h"
#include "interrupt.h"
#include "mem.h"
#include "terminal.h"
#include "thread.h"
#include "uart.h"
#include "loader.h"
#include "syscall.h"
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
  preserve(0x8200000, 0x16000);
  smalloc_init();
  fdt_find_do(dtb_location, "linux,initrd-start", initrd_fdt_callback);
  uart_puts("test_thread\n");
  //test_thread_queue();
  //setup_program_loc(fork_test);
  //thread_create(sys_run_program);
  //schedule();
  //setup_program_loc(check_timer);
  //thread_create(sys_run_program);
  core_timer_enable();
  terminal_run_thread();
  //idle();
  //terminal_run();

  return 0;
}
