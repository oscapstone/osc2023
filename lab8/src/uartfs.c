#include "uartfs.h"
#include "heap.h"
#include "initrd.h"
#include "mem.h"
#include "str.h"
#include "uart.h"
#include <stddef.h>
#include <stdint.h>

extern struct mount *fsRootMount;
extern struct vnode *fsRoot;

struct filesystem *getUartFs(void) {
  struct filesystem *fs = malloc(sizeof(struct filesystem));
  fs->name = "UartFs";
  fs->setup_mount = uartfs_init;
  return fs;
}

/*****************************************************************
 * U cannot do lots operations in the UARTFS.
 ***************************************************************/
int uartfs_init(struct filesystem *fs, struct mount *m) {
  struct vnode *root = m->root;

  root->f_ops =
      (struct file_operations *)malloc(sizeof(struct file_operations));
  root->f_ops->open = uartfs_open;
  root->f_ops->write = uartfs_write;
  root->f_ops->read = uartfs_read;
  root->f_ops->close = uartfs_close;

  char *name = NULL;

  // is not mount at the root of the whole FS
  if (root->internal != NULL) {
    name = ((FsAttr *)root->internal)->name;
  } else {
    name = malloc(sizeof(char) * 16);
    memset(name, 0, 16);
    *name = '/';
  }
  if (root->internal == NULL) {
    root->internal = (FsAttr *)malloc(sizeof(FsAttr));
  }

  FsAttr *attr = root->internal;
  attr->name = name;
  attr->type = DIRTYPE;
  attr->size = 0;
  // Preserve the created dirs
  if (attr->dirs == NULL) {
    attr->dirs = (void *)smalloc(8 * sizeof(void *));
  }
  return 0;
}

int uartfs_open(struct vnode *v, struct file **target) {
  if (*target == NULL) {
    *target = (struct file *)malloc(sizeof(struct file));
  }
  (*target)->vnode = v;
  (*target)->f_pos = 0;
  (*target)->f_ops = v->f_ops;
  return 0;
}

/*************************************************************
 * Write a file in UARTFS == UART send
 ************************************************************/
int uartfs_write(struct file *f, const void *buf, size_t len) {
  const char *c = (const char *)buf;
  int count = 0;
  for (size_t i = 0; i < len; i++) {
    uart_putc(*c++);
    count++;
  }
  return count;
}

/*************************************************************
 * Write a file in UARTFS == UART read
 ************************************************************/
int uartfs_read(struct file *f, void *buf, size_t len) {
  char *c = (char *)buf;
  int count = 0;
  // uart_puth(data);
  for (size_t i = 0; i < len; i++) {
    *c++ = uart_getc();
    count++;
  }
  return count;
}

int uartfs_close(struct file *f) { return 0; }
