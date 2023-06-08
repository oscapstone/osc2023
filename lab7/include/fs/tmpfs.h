#ifndef TMPFS_H
#define TMPFS_H

#include "stddef.h"
#include "fs/vfs.h"

#define FILE_NAME_MAX 16
#define MAX_DIR_ENTRY 17
#define MAX_FILE_SIZE 4096

struct tmpfs_inode
{
    enum node_type type;
    char name[FILE_NAME_MAX];
    struct vnode *entry[MAX_DIR_ENTRY];  // record the vnode under dir_t
    char *data;
    size_t datasize;
};

int register_tmpfs();
int tmpfs_setup_mount(struct filesystem *fs, struct mount *_mount);

int tmpfs_write(struct file *file, const void *buf, size_t len);
int tmpfs_read(struct file *file, void *buf, size_t len);
int tmpfs_open(struct vnode *file_node, struct file **target);
int tmpfs_close(struct file *file);
long tmpfs_getsize(struct vnode *vd);

int tmpfs_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name);
int tmpfs_create(struct vnode *dir_node, struct vnode **target, const char *component_name);
int tmpfs_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name);

struct vnode *tmpfs_create_vnode(struct mount *_mount, enum node_type type);

#endif