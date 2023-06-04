#include "oscos/console.h"
#include "oscos/devicetree.h"
#include "oscos/drivers/aux.h"
#include "oscos/drivers/gpio.h"
#include "oscos/drivers/l1ic.h"
#include "oscos/drivers/l2ic.h"
#include "oscos/drivers/mailbox.h"
#include "oscos/drivers/pm.h"
#include "oscos/initrd.h"
#include "oscos/mem/malloc.h"
#include "oscos/mem/page-alloc.h"
#include "oscos/mem/startup-alloc.h"
#include "oscos/panic.h"
#include "oscos/sched.h"
#include "oscos/shell.h"
#include "oscos/timer/delay.h"
#include "oscos/timer/timeout.h"
#include "oscos/xcpt.h"

static void _foo(void *const _arg) {
  (void)_arg;

  for (int i = 0; i < 10; i++) {
    console_printf("TID %2zu: %d\n", current_thread()->id, i);
    delay_ns(1000000);
    schedule();
  }
}

static void _delayed_shell(void *const _arg) {
  (void)_arg;

  for (int i = 0; i < 20; i++) {
    schedule();
  }
  run_shell();
}

void main(const void *const dtb_start) {
  // Initialize interrupt-related subsystems.
  l1ic_init();
  l2ic_init();
  xcpt_set_vector_table();
  timeout_init();
  XCPT_UNMASK_ALL();

  // Initialize the serial console.
  gpio_init();
  aux_init();
  startup_alloc_init();
  console_init();

  // Initialize the devicetree.
  if (!devicetree_init(dtb_start)) {
    console_puts("WARN: Devicetree blob is invalid.");
  }

  // Initialize the initial ramdisk.
  if (!initrd_init()) {
    console_puts("WARN: Initial ramdisk is invalid.");
  }

  // Initialize the memory allocators.
  page_alloc_init();
  malloc_init();

  // Initialize miscellaneous subsystems.
  mailbox_init();
  pm_init();

  // Initialize the scheduler.

  if (!sched_init()) {
    PANIC("Cannot initialize scheduler: out of memory");
  }

  // Test the scheduler.

  for (int i = 0; i < 32; i++) {
    thread_create(_foo, NULL);
  }
  thread_create(_delayed_shell, NULL);

  idle();
}
