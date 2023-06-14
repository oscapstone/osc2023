#ifndef UARTFS_H
#define UARTFS_H
#define size_t int
#include "vfs.h"

void uartfs_init(struct mount* mount_node);
int uartfs_mount(struct filesystem* fs,struct mount* mount);
int uartfs_write(struct file* file,const void* buf,size_t len);
int uartfs_read(struct file* file,void* buf,size_t len);
int uartfs_open(struct vnode* file_node,struct file** target);
int uartfs_close(struct file* file);

#endif
