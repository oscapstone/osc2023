#include "framefs.h"
#include "heap.h"
#include "initrd.h"
#include "mailbox.h"
#include "mem.h"
#include "str.h"
#include "uart.h"
#include <stddef.h>
#include <stdint.h>

extern struct mount *fsRootMount;
extern struct vnode *fsRoot;

struct filesystem *getFrameFs(void) {
  struct filesystem *fs = malloc(sizeof(struct filesystem));
  fs->name = "FrameFs";
  fs->setup_mount = framefs_init;
  return fs;
}

int framefs_init(struct filesystem *fs, struct mount *m) {
  struct vnode *root = m->root;

  root->f_ops =
      (struct file_operations *)malloc(sizeof(struct file_operations));
  root->f_ops->open = framefs_open;
  root->f_ops->write = framefs_write;
  root->f_ops->read = framefs_read;
  root->f_ops->close = framefs_close;

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
  // Mbox default value
  mbox[0] = 35 * 4;
  mbox[1] = MAILBOX_REQ;

  mbox[2] = 0x48003; // set phy wh
  mbox[3] = 8;
  mbox[4] = 8;
  mbox[5] = 1024; // FrameBufferInfo.width
  mbox[6] = 768;  // FrameBufferInfo.height

  mbox[7] = 0x48004; // set virt wh
  mbox[8] = 8;
  mbox[9] = 8;
  mbox[10] = 1024; // FrameBufferInfo.virtual_width
  mbox[11] = 768;  // FrameBufferInfo.virtual_height

  mbox[12] = 0x48009; // set virt offset
  mbox[13] = 8;
  mbox[14] = 8;
  mbox[15] = 0; // FrameBufferInfo.x_offset
  mbox[16] = 0; // FrameBufferInfo.y.offset

  mbox[17] = 0x48005; // set depth
  mbox[18] = 4;
  mbox[19] = 4;
  mbox[20] = 32; // FrameBufferInfo.depth

  mbox[21] = 0x48006; // set pixel order
  mbox[22] = 4;
  mbox[23] = 4;
  mbox[24] = 1; // RGB, not BGR preferably

  mbox[25] = 0x40001; // get framebuffer, gets alignment on request
  mbox[26] = 8;
  mbox[27] = 8;
  mbox[28] = 4096; // FrameBufferInfo.pointer
  mbox[29] = 0;    // FrameBufferInfo.size

  mbox[30] = 0x40008; // get pitch
  mbox[31] = 4;
  mbox[32] = 4;
  mbox[33] = 0; // FrameBufferInfo.pitch

  mbox[34] = TAG_LAST;
  sys_mailbox_config(CHANNEL_PT, mbox);
  attr->data = (void *)((mbox[28] & 0x3FFFFFFF));
  return 0;
}

int framefs_open(struct vnode *v, struct file **target) {
  if (*target == NULL) {
    *target = (struct file *)malloc(sizeof(struct file));
    memset(*target, 0, sizeof(struct file));
  }
  (*target)->vnode = v;
  (*target)->f_pos = 0;
  (*target)->f_ops = v->f_ops;
  (*target)->data = ((FsAttr *)(v->internal))->data;
  return 0;
}

/*************************************************************
 * Write the frambuffer.
 ************************************************************/
int framefs_write(struct file *f, const void *buf, size_t len) {
  char *c = (char *)buf;
  int count = 0;
  size_t i;
  char *d = f->data;
  uart_puts(" \b"); // Delay, if No this line will break.
  for (i = f->f_pos; i < f->f_pos + len; i++) {
    *(d + i) = *c++;
    count++;
  }
  f->f_pos += len;
  return count;
}

/************************************************************
 * U should never read a Framebuffer
 ***********************************************************/
int framefs_read(struct file *f, void *buf, size_t len) {
  volatile char *c = (char *)buf;
  int count = 0;
  for (size_t i = f->f_pos; i < len; i++) {
    *c++ = uart_getc();
    count++;
  }
  return count;
}

int framefs_ioctl(struct file *f, struct framebuffer_info *fb_info) {
  // Mbox setup value
  mbox[0] = 35 * 4;
  mbox[1] = MAILBOX_REQ;

  mbox[2] = 0x48003; // set phy wh
  mbox[3] = 8;
  mbox[4] = 8;
  mbox[5] = 1024; // FrameBufferInfo.width
  mbox[6] = 768;  // FrameBufferInfo.height

  mbox[7] = 0x48004; // set virt wh
  mbox[8] = 8;
  mbox[9] = 8;
  mbox[10] = 1024; // FrameBufferInfo.virtual_width
  mbox[11] = 768;  // FrameBufferInfo.virtual_height

  mbox[12] = 0x48009; // set virt offset
  mbox[13] = 8;
  mbox[14] = 8;
  mbox[15] = 0; // FrameBufferInfo.x_offset
  mbox[16] = 0; // FrameBufferInfo.y.offset

  mbox[17] = 0x48005; // set depth
  mbox[18] = 4;
  mbox[19] = 4;
  mbox[20] = 32; // FrameBufferInfo.depth

  mbox[21] = 0x48006; // set pixel order
  mbox[22] = 4;
  mbox[23] = 4;
  mbox[24] = 1; // RGB, not BGR preferably

  mbox[25] = 0x40001; // get framebuffer, gets alignment on request
  mbox[26] = 8;
  mbox[27] = 8;
  mbox[28] = 4096; // FrameBufferInfo.pointer
  mbox[29] = 0;    // FrameBufferInfo.size

  mbox[30] = 0x40008; // get pitch
  mbox[31] = 4;
  mbox[32] = 4;
  mbox[33] = 0; // FrameBufferInfo.pitch

  mbox[34] = TAG_LAST;
  sys_mailbox_config(CHANNEL_PT, mbox);
  // Restore the attribute back
  fb_info->width = mbox[5];
  fb_info->height = mbox[6];
  fb_info->pitch = mbox[33];
  fb_info->isrgb = mbox[24];
  f->data = (void *)((unsigned int)mbox[28] & 0x3FFFFFFF);
}

int framefs_close(struct file *f) { return 0; }
