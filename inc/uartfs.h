#ifndef _UARTFS_H
#define _UARTFS_H

#include <vfs.h>

struct uartfs_internal{
    const char *name;
    struct vnode oldnode;
};

int uartfs_mount(struct filesystem *fs, struct mount *mount);
int uartfs_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name);
int uartfs_create(struct vnode *dir_node, struct vnode **target, const char *component_name);
int uartfs_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name);
int uartfs_isdir(struct vnode *dir_node);
int uartfs_getname(struct vnode *dir_node, const char **name);
int uartfs_getsize(struct vnode *dir_node);
int uartfs_write(struct file *file, const void *buf, size_t len);
int uartfs_read(struct file *file, void *buf, size_t len);
int uartfs_open(struct vnode *file_node, struct file *target);
int uartfs_close(struct file *file);
long uartfs_lseek64(struct file *file, long offset, int whence);
int uartfs_ioctl(struct file *file, uint64 request, va_list args);
int uartfs_sync(struct filesystem *fs);

struct filesystem *uartfs_init(void);

#endif