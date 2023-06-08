#ifndef VFS_H
#define VFS_H

#include "mem.h"
#include "str.h"
#include <stddef.h>
#include <stdint.h>
typedef enum {
  DIRTYPE = 0,
  NORMAL,
} FsTy;

/* THis is the INODE*/
struct vnode {
  struct vnode *parent; // Parent DIR
  struct mount *mount;	// The mount node of the FS
  struct vnode_operations *v_ops; // Vnode operations
  struct file_operations *f_ops; // File operations
  char name[16];	// Name of the node.
  FsTy type;		// Type of the vnode
  void *internal; // The for each file defined
};

/* Mount Node of the FS*/
struct mount {
  struct vnode *root;		// The root of this fs
  struct filesysytem *system;	// Point to the file system
};

/* Opening a file need to do following things.  * 1. Allocat a file strucure.
 * * 2. Copy the member operation from dentry (INODE) A place the file
 * descriptor table for the process.
 */
struct file {
  struct vnode *vnode;	// The original vnode of file
  size_t f_pos;		// Current position of the file
  size_t Eof;		// End of the FILE
  struct file_operations *f_ops;
  int flags;
  void *data;		// The data of this file
};

/* This structure contain the filesystem name such as Ext3
 * And 	The setup_mount function which will be called by the VFS to mount
 * the file system.
 */
struct filesystem {
  const char *name;
  int (*setup_mount)(struct filesystem *fs, struct mount *mount);
};

/* Some vnode operations */
struct vnode_operations {
  int (*lookup)(struct vnode *dir_node, struct vnode **target,
                const char *component_name);
  int (*create)(struct vnode *dir_node, struct vnode **target,
                const char *component_name);
  int (*mkdir)(struct vnode *dir_node, struct vnode **target,
               const char *component_name);
};

struct file_operations {
  int (*write)(struct file *, const void *buf, int len);
  int (*read)(struct file *, void *buf, int len);
  int (*open)(struct vnode *, struct file **); // open vnode to table
  int (*close)(struct file *);
};

// struct mount* rootfs;

/* From linux vfs.rst It sys.
 *
 */
int register_filesystem(struct filesystem *fs);
int vfs_open(const char *pathname, int flags, struct file **target,
             struct vnode *root);
int vfs_close(struct file *file);
int vfs_read(struct file *file, void *buf, size_t len);
int vfs_write(struct file *file, const void *buf, size_t len);

int vfs_mkdir(char *pathname, struct vnode *root);
int vfs_create(struct vnode *dir_node, struct vnode **target, const char *name);

int vfs_mount(const char *target, const char *filesystem);
/* To look up an inode need VFS xall `lookup()`of the parent DIR of the
 * inode. We need to get the Dentry -> Inode. then we can do `open()` ...
 */
int vfs_lookup(const char *pathname, struct vnode **target, struct vnode *root);


#define O_CREAT 0b0100
#define SEEK_SET 0

// Helper
char *getFileName(char *dest, const char *from);
int vfs_getLastDir(char *path, struct vnode *dir, struct vnode **t);
struct vnode *vfs_reWritePath(char *pathName, struct vnode *dir,
                              char **new_pathName);

#endif // VFS_H
