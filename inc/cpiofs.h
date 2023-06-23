#ifndef _CPIOFS_H
#define _CPIOFS_H

#include <list.h>
#include <vfs.h>

#define CPIO_TYPE_MASK  0060000
#define CPIO_TYPE_DIR   0040000
#define CPIO_TYPE_FILE  0000000

#define CPIOFS_TYPE_UNDEFINE     0x0
#define CPIOFS_TYPE_FILE         0x1
#define CPIOFS_TYPE_DIR          0x2

struct cpiofs_file_t {
    const char *data;
    int size;
};

struct cpiofs_dir_t {
    /* Link cpiofs_internal */
    struct list_head list;
};

struct cpiofs_internal {
    const char *name;
    int type;
    union {
        struct cpiofs_file_t file;
        struct cpiofs_dir_t dir;
    };
    struct vnode *node;
    struct list_head list;
};

int cpiofs_mount(struct filesystem *fs, struct mount *mount);
int cpiofs_lookup(struct vnode *dir_node, struct vnode **target,
                        const char *component_name);
int cpiofs_create(struct vnode *dir_node, struct vnode **target,
                        const char *component_name);
int cpiofs_mkdir(struct vnode *dir_node, struct vnode **target,
                       const char *component_name);
int cpiofs_isdir(struct vnode *dir_node);
int cpiofs_getname(struct vnode *dir_node, const char **name);
int cpiofs_getsize(struct vnode *dir_node);
int cpiofs_write(struct file *file, const void *buf, size_t len);
int cpiofs_read(struct file *file, void *buf, size_t len);
int cpiofs_open(struct vnode *file_node, struct file *target);
int cpiofs_close(struct file *file);
long cpiofs_lseek64(struct file *file, long offset, int whence);

struct filesystem *cpiofs_init(void);

#endif