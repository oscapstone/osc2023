#ifndef __DEVFS_H_
#define __DEVFS_H_

#include "vfs.h"
extern struct filesystem devfs;

// fops
int devfs_write(struct file *file, void *buf, size_t len);
int devfs_read(struct file *file, void *buf, size_t len);
int devfs_open(struct vnode *file_node, struct file **target);
int devfs_close(struct file *file);
long devfs_lseek(struct file *file, long offset, int whence);

// vops
int devfs_mkdir(struct vnode *dir_node, struct vnode **target, char *component_name);
int devfs_create(struct vnode *dir_node, struct vnode **target, char *component_name);
int devfs_lookup(struct vnode *dir_node, struct vnode **target, char *component_name);

int devfs_setup_mount(struct filesystem *fs, struct mount *mount);

#endif // __DEVFS_H_