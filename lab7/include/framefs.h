#ifndef UARTFS_H
#define UARTFS_H

#include "mem.h"
#include "ramfs.h"
#include "str.h"
#include "uart.h"
#include "vfs.h"

struct framebuffer_info{
	unsigned int width;
	unsigned int height;
	unsigned int pitch;
	unsigned int isrgb;
};

int framefs_init(struct filesystem *fs, struct mount *m);

struct filesystem *getFrameFs();

int framefs_read(struct file *f, void *buf, size_t len);
int framefs_write(struct file *f, const void *buf, size_t len);
int framefs_open(struct vnode *, struct file **target);
int framefs_close(struct file *f);
int framefs_ioctl(struct file *f, struct framebuffer_info *fb_info);

#endif
