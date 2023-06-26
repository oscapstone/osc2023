#ifndef __TMPFS_H
#define __TMPFS_H

#include "vfs.h"

#define TMPFS_MAX_CONTENT_SIZE 4096

struct tmpfs_vinfo {
    char name[16];
    int namesize;
    int filesize;
    // make this pointer
    // we may store the absolute address from initramfs
    char *content;
};
int mount_tmpfs(struct filesystem *fs, struct mount *mnt);

int tmpfs_lookup(struct vnode* dir_node, struct vnode** target, const char * componenet_name);

int tmpfs_create(struct vnode* dir_node, struct vnode** target, const char *component_name);

int tmpfs_mkdir(struct vnode* dir_node, struct vnode** target, const char *componenet_name);
int tmpfs_write(struct file* file, const void *buf, size_t len);
int tmpfs_read(struct file* file, const void *buf, size_t len);
int tmpfs_open(struct vnode* file_node, struct file** target);
int tmpfs_close(struct file* file);
long tmpfs_lseek64(struct file* file, long offset, int whence);
void tmpfs_print_name(struct vnode* node);
void init_fs();
struct vnode* new_tmpfs_node(const char *name, struct vnode *dir_node);
int init_tmpfs(struct filesystem *fs);
int tmpfs_stat(struct file *f, struct fstat *stat);
#endif