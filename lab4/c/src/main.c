#include "oscos/console.h"
#include "oscos/devicetree.h"
#include "oscos/drivers/aux.h"
#include "oscos/drivers/gpio.h"
#include "oscos/drivers/l1ic.h"
#include "oscos/drivers/l2ic.h"
#include "oscos/drivers/mailbox.h"
#include "oscos/drivers/pm.h"
#include "oscos/initrd.h"
#include "oscos/shell.h"
#include "oscos/timer/timeout.h"
#include "oscos/xcpt.h"

void main(const void *const dtb_start) {
  l1ic_init();
  l2ic_init();
  xcpt_set_vector_table();
  timeout_init();
  XCPT_UNMASK_ALL();

  gpio_init();
  aux_init();
  console_init();

  mailbox_init();
  pm_init();

  if (!devicetree_init(dtb_start)) {
    console_puts("WARN: Devicetree blob is invalid.");
  }

  if (!initrd_init()) {
    console_puts("WARN: Initial ramdisk is invalid.");
  }

  run_shell();
}
