#ifndef _VFS_H_
#define _VFS_H_

#include "stddef.h"

#define MAX_PATH_NAME 255 // The maximum length of a path name
#define MAX_FD 16         // The maximum number of file descriptors (file descriptors) that each process can open
#define O_CREAT 00000100  // Indicates one of the flags when opening a file. When this flag is specified, a new file is created if the file does not exist.
#define SEEK_SET 0        // Indicates the starting position of the file read and write position indicator. In this implementation, is set to 0, which means read and write operations from the beginning of the file.
#define MAX_FS_REG 0x50   // Indicates the maximum number of registered file systems.
#define MAX_DEV_REG 0x10  // Indicates the maximum number of registered devices

// Defines two types of file system nodes: 
// 1. directories (dir_t)
// 2. files (file_t).
enum fsnode_type
{
    dir_t,
    file_t
};

// Node in the file system
struct vnode
{
    struct mount *mount;            // Superblock        : represents mounted fs
    struct vnode_operations *v_ops; // inode & dentry Ops: represents kernel methods for vnode
    struct file_operations *f_ops;  // file Ops          : represents process methods for opened file
    void *internal;                 // vnode itself      : directly point to fs's vnode
};

// File handle in the file system
struct file
{
    struct vnode *vnode;
    size_t f_pos; // RW position of this file handle
    struct file_operations *f_ops;
    int flags;
};

// Mount point in the file system
struct mount
{
    struct vnode *root;     // the root node of the mount point
    struct filesystem *fs;  // the mounted file system.
};

struct filesystem
{
    const char *name;  // : The name of the file system.
    int (*setup_mount)(struct filesystem *fs, struct mount *mount); // A function pointer for initializing the mount point of the file system.
};

struct file_operations
{
    int (*write)(struct file *file, const void *buf, size_t len);
    int (*read)(struct file *file, void *buf, size_t len);
    int (*open)(struct vnode *file_node, struct file **target);
    int (*close)(struct file *file);
    long (*lseek64)(struct file *file, long offset, int whence);
    long (*getsize)(struct vnode *vd);
};

struct vnode_operations
{
    int (*lookup)(struct vnode *dir_node, struct vnode **target,
                  const char *component_name);
    int (*create)(struct vnode *dir_node, struct vnode **target,
                  const char *component_name);
    int (*mkdir)(struct vnode *dir_node, struct vnode **target,
                 const char *component_name);
};

int register_filesystem(struct filesystem *fs);
int register_dev(struct file_operations* fo);
struct filesystem *find_filesystem(const char *fs_name);
int vfs_open(const char *pathname, int flags, struct file **target);
int vfs_close(struct file *file);
int vfs_write(struct file *file, const void *buf, size_t len);
int vfs_read(struct file *file, void *buf, size_t len);
int vfs_mkdir(const char *pathname);
int vfs_mount(const char *target, const char *filesystem);
int vfs_lookup(const char *pathname, struct vnode **target);
int vfs_mknod(char* pathname, int id);

void init_rootfs();
void vfs_test();
char* get_absolute_path(char* path,char* curr_working_dir);

#endif /* _VFS_H_ */
