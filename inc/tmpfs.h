#ifndef _TMPFS_H
#define _TMPFS_H

#include <vfs.h>

#define TMPFS_FILE_MAXSIZE      PAGE_SIZE
#define TMPFS_DIR_MAXSIZE       0x10
#define TMPFS_TYPE_UNDEFINE     0x0
#define TMPFS_TYPE_FILE         0x1
#define TMPFS_TYPE_DIR          0x2
#define TMPFS_NAME_MAXLEN       0x10

struct tmpfs_file_t{
    char *data;
    int size;
    int capacity;
};

struct  tmpfs_dir_t{
    int size;
    struct vnode *entries[TMPFS_DIR_MAXSIZE];
};

struct tmpfs_internal{
    char name[TMPFS_NAME_MAXLEN];
    int type;
    union {
        struct tmpfs_file_t *file;
        struct tmpfs_dir_t *dir;
    };
    struct vnode *oldnode;
};

int tmpfs_mount(struct filesystem *fs, struct mount *mount);
int tmpfs_alloc_vnode(struct filesystem *fs, struct vnode **target);
int tmpfs_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name);
int tmpfs_create(struct vnode *dir_node, struct vnode **target, const char *component_name);
int tmpfs_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name);
int tmpfs_isdir(struct vnode *dir_node);
int tmpfs_getname(struct vnode *dir_node, const char **name);
int tmpfs_getsize(struct vnode *dir_node);
int tmpfs_write(struct file *file, const void *buf, size_t len);
int tmpfs_read(struct file *file, void *buf, size_t len);
int tmpfs_open(struct vnode *file_node, struct file *target);
int tmpfs_close(struct file *file);
long tmpfs_lseek64(struct file *file, long offset, int whence);
int tmpfs_ioctl(struct file *file, uint64 request, va_list args);
int tmpfs_sync(struct filesystem *fs);

struct filesystem *tmpfs_init(void);

#endif