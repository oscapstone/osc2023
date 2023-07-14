#ifndef OSCOS_FS_VFS_H
#define OSCOS_FS_VFS_H

#include <stddef.h>

#include "oscos/uapi/fcntl.h" // O_* constants.

struct vnode {
  struct mount *mount;
  struct vnode_operations *v_ops;
  struct file_operations *f_ops;
  void *internal;
};

// file handle
struct file {
  struct vnode *vnode;
  size_t f_pos; // RW position of this file handle
  struct file_operations *f_ops;
  int flags;
};

struct mount {
  struct vnode *root;
  struct filesystem *fs;
};

struct filesystem {
  const char *name;
  int (*setup_mount)(struct filesystem *fs, struct mount *mount);
};

struct device {
  const char *name;
  int (*setup_mount)(struct device *dev, struct vnode *vnode);
};

struct file_operations {
  int (*write)(struct file *file, const void *buf, size_t len);
  int (*read)(struct file *file, void *buf, size_t len);
  int (*open)(struct vnode *file_node, struct file **target);
  int (*close)(struct file *file);
  long (*lseek64)(struct file *file, long offset, int whence);
  int (*ioctl)(struct file *file, unsigned long request, void *payload);
};

struct vnode_operations {
  int (*lookup)(struct vnode *dir_node, struct vnode **target,
                const char *component_name);
  int (*create)(struct vnode *dir_node, struct vnode **target,
                const char *component_name);
  int (*mkdir)(struct vnode *dir_node, struct vnode **target,
               const char *component_name);
  int (*mknod)(struct vnode *dir_node, struct vnode **target,
               const char *component_name, struct device *device);
  long (*get_size)(struct vnode *vnode);
};

extern struct mount rootfs;

int register_filesystem(struct filesystem *fs);
int register_device(struct device *dev);

int vfs_open(const char *pathname, int flags, struct file **target);
int vfs_open_relative(struct vnode *cwd, const char *pathname, int flags,
                      struct file **target);
int vfs_close(struct file *file);
int vfs_write(struct file *file, const void *buf, size_t len);
int vfs_read(struct file *file, void *buf, size_t len);
long vfs_lseek64(struct file *file, long offset, int whence);
int vfs_ioctl(struct file *file, unsigned long request, void *payload);

int vfs_mkdir(const char *pathname);
int vfs_mkdir_relative(struct vnode *cwd, const char *pathname);
int vfs_mount(const char *target, const char *filesystem);
int vfs_mount_relative(struct vnode *cwd, const char *target,
                       const char *filesystem);
int vfs_lookup(const char *pathname, struct vnode **target);
int vfs_lookup_relative(struct vnode *cwd, const char *pathname,
                        struct vnode **target);

int vfs_mknod(const char *target, const char *device);

typedef struct {
  struct file *file;
  size_t refcnt;
} shared_file_t;

shared_file_t *shared_file_new(struct file *file);
shared_file_t *shared_file_clone(shared_file_t *shared_file);
void shared_file_drop(shared_file_t *shared_file);

#endif
