#include "oscos/console-dev.h"
#include "oscos/console.h"
#include "oscos/devicetree.h"
#include "oscos/drivers/aux.h"
#include "oscos/drivers/gpio.h"
#include "oscos/drivers/l1ic.h"
#include "oscos/drivers/l2ic.h"
#include "oscos/drivers/mailbox.h"
#include "oscos/drivers/pm.h"
#include "oscos/drivers/sdhost.h"
#include "oscos/framebuffer-dev.h"
#include "oscos/fs/initramfs.h"
#include "oscos/fs/sd-fat32.h"
#include "oscos/fs/tmpfs.h"
#include "oscos/fs/vfs.h"
#include "oscos/initrd.h"
#include "oscos/mem/malloc.h"
#include "oscos/mem/page-alloc.h"
#include "oscos/mem/startup-alloc.h"
#include "oscos/mem/vm/kernel-page-tables.h"
#include "oscos/panic.h"
#include "oscos/sched.h"
#include "oscos/shell.h"
#include "oscos/timer/delay.h"
#include "oscos/timer/timeout.h"
#include "oscos/xcpt.h"

static void _run_shell(void *const _arg) {
  (void)_arg;
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
  vm_setup_finer_granularity_linear_mapping();

  // Initialize miscellaneous subsystems.
  mailbox_init();
  pm_init();

  // Initialize the scheduler.

  if (!sched_init()) {
    PANIC("Cannot initialize scheduler: out of memory");
  }

  // Initialize VFS.

  sd_init();

  int vfs_op_result;

  vfs_op_result = register_filesystem(&tmpfs);
  if (vfs_op_result < 0)
    PANIC("Cannot register tmpfs: errno %d", -vfs_op_result);
  vfs_op_result = register_filesystem(&initramfs);
  if (vfs_op_result < 0)
    PANIC("Cannot register initramfs: errno %d", -vfs_op_result);
  vfs_op_result = register_filesystem(&sd_fat32);
  if (vfs_op_result < 0)
    PANIC("Cannot register fat32: errno %d", -vfs_op_result);
  vfs_op_result = register_device(&console_dev);
  if (vfs_op_result < 0)
    PANIC("Cannot register console device: errno %d", -vfs_op_result);
  vfs_op_result = register_device(&framebuffer_dev);
  if (vfs_op_result < 0)
    PANIC("Cannot register framebuffer device: errno %d", -vfs_op_result);

  vfs_op_result = tmpfs.setup_mount(&tmpfs, &rootfs);
  if (vfs_op_result < 0)
    PANIC("Cannot setup root file system: errno %d", -vfs_op_result);

  vfs_op_result = vfs_mkdir("/initramfs");
  if (vfs_op_result < 0)
    PANIC("Cannot mkdir /initramfs: errno %d", -vfs_op_result);
  vfs_op_result = vfs_mount("/initramfs", "initramfs");
  if (vfs_op_result < 0)
    PANIC("Cannot mount initramfs on /initramfs: errno %d", -vfs_op_result);

  vfs_op_result = vfs_mkdir("/dev");
  if (vfs_op_result < 0)
    PANIC("Cannot mkdir /dev: errno %d", -vfs_op_result);
  vfs_op_result = vfs_mknod("/dev/uart", "console");
  if (vfs_op_result < 0)
    PANIC("Cannot mknod /dev/uart: errno %d", -vfs_op_result);
  vfs_op_result = vfs_mknod("/dev/framebuffer", "framebuffer");
  if (vfs_op_result < 0)
    PANIC("Cannot mknod /dev/framebuffer: errno %d", -vfs_op_result);

  vfs_op_result = vfs_mkdir("/boot");
  if (vfs_op_result < 0)
    PANIC("Cannot mkdir /boot: errno %d", -vfs_op_result);
  vfs_op_result = vfs_mount("/boot", "fat32");
  if (vfs_op_result < 0)
    PANIC("Cannot mount fat32 on /boot: errno %d", -vfs_op_result);

  thread_create(_run_shell, NULL);

  sched_setup_periodic_scheduling();
  idle();
}
