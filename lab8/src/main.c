#include "dtb.h"
#include "exception.h"
#include "framefs.h"
#include "heap.h"
#include "initrd.h"
#include "interrupt.h"
#include "loader.h"
#include "mem.h"
#include "ramfs.h"
#include "syscall.h"
#include "terminal.h"
#include "thread.h"
#include "uart.h"
#include "uartfs.h"
#include "vfs.h"
#include <stdint.h>

extern void set_exception_vector_table(void);
extern struct vnode *fsRoot;
extern struct mount *fsRootMount;
struct file *uart_stdin = NULL;
struct file *uart_stdout = NULL;
struct file *uart_stderr = NULL;

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
  // Initial Fs
  fsRootMount = (struct mount *)malloc(sizeof(struct mount));
  memset(fsRootMount, 0, sizeof(struct mount));
  fsRootMount->root = (struct vnode *)malloc(sizeof(struct vnode));
  fsRoot = fsRootMount->root;
  memset(fsRoot, 0, sizeof(struct vnode));
  fsRoot->parent = NULL;
  fsRoot->mount = fsRootMount;
  struct filesystem *fs = getRamFs();
  register_filesystem(fs);
  fs->setup_mount(fs, fsRootMount);
  // Mount the Ramfs
  vfs_mkdir("initramfs", NULL);
  struct vnode *test;
  vfs_lookup("initramfs", &test, NULL);
  ramfs_initFsCpio(test);

  // InitUartFs
  vfs_mkdir("/dev", NULL);
  vfs_mkdir("/dev/uart", NULL);
  struct filesystem *ufs = getUartFs();
  register_filesystem(ufs);
  vfs_lookup("/dev/uart", &test, NULL);
  struct mount *uartM = (struct mount *)malloc(sizeof(struct mount));
  uartM->root = test;
  test->mount = uartM;
  ufs->setup_mount(test, uartM);
  vfs_open("/dev/uart", 0, &uart_stdin, NULL);
  vfs_open("/dev/uart", 0, &uart_stdout, NULL);
  vfs_open("/dev/uart", 0, &uart_stderr, NULL);

  // InitFramFs
  vfs_mkdir("/dev/framebuffer", NULL);
  struct filesystem *ffs = getFrameFs();
  register_filesystem(ffs);
  vfs_lookup("/dev/framebuffer", &test, NULL);
  struct mount *frameM = (struct mount *)malloc(sizeof(struct mount));
  frameM->root = test;
  test->mount = frameM;
  ffs->setup_mount(test, frameM);


  // FAT
  struct filesystem *fatfs = getFatFs();
  register_filesystem(fatfs);
  vfs_mkdir("/boot", NULL);
  vfs_lookup("/boot", &test, NULL);
  struct mount *fatM = (struct mount *)malloc(sizeof(struct mount));
  fatM->root = test;
  test->mount = fatM;
  fatfs->setup_mount(test, fatM);

  ramfs_dump(fsRoot, 0);
  core_timer_enable();
  terminal_run_thread();

  return 0;
}
