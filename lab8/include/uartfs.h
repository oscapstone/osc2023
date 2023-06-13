#ifndef UARTFS_H
#define UARTFS_H

#include "mem.h"
#include "ramfs.h"
#include "str.h"
#include "uart.h"
#include "vfs.h"

int uartfs_init(struct filesystem *fs, struct mount *m);

struct filesystem *getUartFs();

int uartfs_read(struct file *f, void *buf, size_t len);
int uartfs_write(struct file *f, const void *buf, size_t len);
int uartfs_open(struct vnode *, struct file **target);
int uartfs_close(struct file *f);

#endif
