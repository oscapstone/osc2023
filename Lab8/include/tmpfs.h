#ifndef _TMPFS_H
#define _TMPFS_H

#include "vfs.h"

#define EXISTED 0
#define NOTFOUND -1
#define NOTDIR -2

/* mount operations */
int tmpfs_mount(struct filesystem *fs, struct mount *mount);

/* vnode operations */
int tmpfs_lookup(struct vnode *dir_node, struct vnode **target, char *component_name);
int tmpfs_create(struct vnode *dir_node, struct vnode **target, char *component_name);
int tmpfs_mkdir(struct vnode *dir_node, struct vnode **target, char *component_name);

/* file operations */
int tmpfs_write(struct file *file, void *buf, size_t len);
int tmpfs_read(struct file *file, void *buf, size_t len);
int tmpfs_open(struct vnode *file_node, struct file **target);
int tmpfs_close(struct file *file);

int initramfs_mount(filesystem_t *fs, mount_t *mount);
void init_cpio();

/* vnode operations */
int initramfs_lookup(struct vnode *dir_node, struct vnode **target, char *component_name);
int initramfs_create(struct vnode *dir_node, struct vnode **target, char *component_name);
int initramfs_mkdir(struct vnode *dir_node, struct vnode **target, char *component_name);

/* file operations */
int initramfs_write(struct file *file, void *buf, size_t len);
int initramfs_read(struct file *file, void *buf, size_t len);
int initramfs_open(struct vnode *file_node, struct file **target);
int initramfs_close(struct file *file);

#endif