#ifndef _FRAMEBUFFERFS_H
#define _FRAMEBUFFERFS_H

#include <vfs.h>
#include <type.h>

struct fb_info{
    uint32 width;
    uint32 height;
    uint32 pitch;
    uint32 isrgb;
};

struct fbfs_internal{
    const char *name;
    struct vnode oldnode;
    uint8 *lfb;
    uint32 lfbsize;
    int isopened;
    int isinit;
};

int fbfs_mount(struct filesystem *fs, struct mount *mount);
int fbfs_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name);
int fbfs_create(struct vnode *dir_node, struct vnode **target, const char *component_name);
int fbfs_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name);
int fbfs_isdir(struct vnode *dir_node);
int fbfs_getname(struct vnode *dir_node, const char **name);
int fbfs_getsize(struct vnode *dir_node);
int fbfs_write(struct file *file, const void *buf, size_t len);
int fbfs_read(struct file *file, void *buf, size_t len);
int fbfs_open(struct vnode *file_node, struct file *target);
int fbfs_close(struct file *file);
long fbfs_lseek64(struct file *file, long offset, int whence);
int fbfs_ioctl(struct file *file, uint64 request, va_list args);
int fbfs_sync(struct filesystem *fs);

struct filesystem *framebufferfs_init(void);

#endif