#ifndef _TMPFS_H
#define _TMPFS_H
#include "vfs.h"
#include "utils.h"
#include "list.h"


struct tmpfs_file_node {
    size_t file_size;
    size_t buffer_size;
    struct vnode *vnode; //corresponding vnode
    char *content_start;
};

extern struct file_operations tmpfs_f_ops;
extern struct vnode_operations tmpfs_v_ops;
extern int tmpfs_setup_mount(struct filesystem* fs, struct mount* mount);
extern int tmpfs_write(struct file* file, const void* buf, size_t len);
extern int tmpfs_read(struct file* file, void* buf, size_t len);
extern int tmpfs_open(struct vnode* file_node, struct file** target);
extern int tmpfs_close(struct file* file);
extern long tmpfs_lseek64(struct file* file, long offset, int whence);

extern int tmpfs_lookup(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
extern int tmpfs_create(struct vnode* dir_node, struct vnode** target,
            const char* component_name);
extern int tmpfs_mkdir(struct vnode* dir_node, struct vnode** target,
            const char* component_name);
#endif