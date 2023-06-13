#ifndef _TMPFS_H
#define _TMPFS_H

#include "vfs.h"

#define TMPFS_COMP_LEN      16
#define MAX_DIRENT          16
#define MAX_FILESIZE        0x1000

struct tmpfs_internal {
    char name[TMPFS_COMP_LEN];
    int type;
    struct tmpfs_internal *parent;
    struct tmpfs_internal *child[MAX_DIRENT];
    struct vnode *vnode;
    int size; // use for child count and data size
    void *data;
};

int tmpfs_setup_mount();
int tmpfs_register();

struct vnode* tmpfs_new_node(struct tmpfs_internal *parent, const char *name, int type);

int tmpfs_open(struct vnode *file_node, struct file **target);
int tmpfs_close(struct file *file);
int tmpfs_write(struct file *file, const void *buf, unsigned len);
int tmpfs_read(struct file *file, void *buf, unsigned len);
int tmpfs_create(struct vnode *dir_node, struct vnode **target, const char *component_name);
int tmpfs_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name);
int tmpfs_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name);

#endif