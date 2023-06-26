#ifndef __VFS_H
#define __VFS_H

#define O_CREAT 000000100
#define O_READ 000000010
#define O_WRITE 000000001

#define VFS_DIR 0
#define VFS_FILE 1

# define SEEK_SET 0
#define VFS_NOT_FOUND -1
#define VFS_NOT_DIR -2
#define VFS_MOUNTED -3
#define VFS_CAN_CREATE -4
#define VFS_EXIST -5
#define VFS_NOT_SUPPORT -6
#define VFS_CANNOT_OPEN_DIR -7
#define HANDLE_NOT_USED -1

#include "ds/list.h"
typedef unsigned long size_t;
typedef void (*apply_dir_func)(struct vnode*);

struct vnode {
  // if mount is not NULL
  // that means the vnode is being mounted
  // we can use and must use the mount->root as new vnode for operations
  struct mount* mount;
  struct vnode* parent;
  struct vnode_operations* v_ops;
  struct file_operations* f_ops;
  // list for store child vnode
  // should only be used if vnode is for directory
  struct ds_list_head ch_list;
  // list for being link to be as child
  struct ds_list_head v_head;
  int flag;
  void* internal;
};

// file handle
struct file {
  struct vnode* vnode;
  size_t f_pos;  // RW position of this file handle
  struct file_operations* f_ops;
  int flags;
};

struct fstat {
  long filesize;
  long namesize;
};

struct mount {
  struct vnode* root;
  struct vnode* mountpoint;
  struct filesystem* fs;
};

struct filesystem {
  const char* name;
  struct vnode *root;
  int (*init_fs)(struct filesystem *fs);
  int (*setup_mount)(struct filesystem* fs, struct mount* mount);
  struct ds_list_head fs_head;
};

struct file_operations {
  int (*write)(struct file* file, const void* buf, size_t len);
  int (*read)(struct file* file, void* buf, size_t len);
  int (*open)(struct vnode* file_node, struct file** target);
  int (*close)(struct file* file);
  int (*stat)(struct file *file, struct fstat *stat);
  long (*lseek64)(struct file* file, long offset, int whence);
  int (*ioctl)(struct file *file, unsigned long request, void *buf);
};

struct vnode_operations {
  int (*lookup)(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
  int (*create)(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
  int (*mkdir)(struct vnode* dir_node, struct vnode** target,
              const char* component_name);
};


// register the file system to the kernel.
// you can also initialize memory pool of the file system here.
int register_filesystem(struct filesystem* fs);

// 1. Lookup pathname
// 2. Create a new file handle for this vnode if found.
// 3. Create a new file if O_CREAT is specified in flags and vnode not found
// lookup error code shows if file exist or not or other error occurs
// 4. Return error code if fails
int vfs_open(const char* pathname, int flags, struct file** target);

// 1. release the file handle
// 2. Return error code if fails
int vfs_close(struct file* file);

// 1. write len byte from buf to the opened file.
// 2. return written size or error code if an error occurs.
int vfs_write(struct file* file, const void* buf, size_t len);

// 1. read min(len, readable size) byte to buf from the opened file.
// 2. block if nothing to read for FIFO type
// 2. return read size or error code if an error occurs.
int vfs_read(struct file* file, void* buf, size_t len);
int vfs_mkdir(const char* pathname);
int vfs_mount(const char* target, const char* filesystem);
int vfs_lookup(const char* pathname, struct vnode** target);

struct filesystem *find_fs_by_name(const char *name);
void vfs_apply_dir(struct vnode* dir_node, apply_dir_func func);
int vfs_create(const char *pathname, int flags, struct vnode** target);
int vfs_stat(struct file *f, struct fstat *stat);
int vfs_lseek64(struct file *f, long offset, int whence);
int vops_not_support();
int fops_not_support();
#endif