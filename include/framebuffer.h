#ifndef __FRAMEBUFFER_H
#define __FRAMEBUFFER_H

int init_framebuffer(struct filesystem* fs);
int framebuffer_setup_mount(struct filesystem* fs, struct mount *mnt);
int framebuffer_close(struct file *f);
int framebuffer_lseek64(struct file *f, int offset, int whence);
int framebuffer_open(struct vnode *file_node, struct file **target);
int framebuffer_write(struct file *f, void *buf, long len);
int framebuffer_ioctl(struct file *f, unsigned long request, void *buf);
void setup_dev_framebuffer();
#endif