#ifndef _INITRAMFS_H
#define _INITRAMFS_H

#include "vfs.h"

#define INITRAMFS_COMP_LEN      16
#define MAX_DIRENT              16
#define MAX_FILESIZE            0x1000

struct initramfs_internal {
    char name[INITRAMFS_COMP_LEN];
    int type;
    struct initramfs_internal *parent;
    struct initramfs_internal *child[MAX_DIRENT];
    struct vnode *vnode;
    int size; // use for child count and data size
    void *data;
};

int initramfs_setup_mount(struct filesystem* fs, struct mount* mount);
int initramfs_register();

struct vnode* initramfs_new_node(struct initramfs_internal *parent, const char *name, int type);

int initramfs_open(struct vnode *file_node, struct file **target);
int initramfs_close(struct file *file);
int initramfs_write(struct file *file, const void *buf, unsigned len);
int initramfs_read(struct file *file, void *buf, unsigned len);
int initramfs_create(struct vnode *dir_node, struct vnode **target, const char *component_name);
int initramfs_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name);
int initramfs_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name);

void parse_initramfs();

#endif