#ifndef VFS_H
#define VFS_H

#include "str.h"
#include "mem.h"
#include <stdint.h>
#include <stddef.h>
typedef enum{
	DIRTYPE = 0,
	NORMAL,
}FsTy;

/* THis is the INODE*/
struct vnode{
	struct vnode* parent;
	struct mount* mount;
	struct vnode_operations* v_ops;
	struct file_operations* f_ops;
	char name[16];
	FsTy type;	
	void* internal; // The for each file defined
};

struct mount{
	struct vnode* root;
	struct filesysytem *system;
};

/* Opening a file need to do following things.  * 1. Allocat a file strucure.  * 2. Copy the member operation from dentry (INODE)
 * A place the file descriptor table for the process.
 */
struct file{
	struct vnode *vnode;
	size_t f_pos;
	struct file_operations *f_ops;
	int flags;
};

/* This structure contain the filesystem name such as Ext3
 * And 	The setup_mount function which will be called by the VFS to mount
 * the file system.
 */
struct filesystem{
	const char* name;
	int (*setup_mount)(struct filesystem *fs, struct mount* mount);
};

/* Some vnode operations */
struct vnode_operations {
	int (*lookup)(struct vnode* dir_node, struct vnode** target,
			const char* component_name);
	int (*create)(struct vnode *dir_node, struct vnode** target,
			const char* component_name);
	int (*mkdir)(struct vnode *dir_node, struct vnode** target,
			const char* component_name);
};

struct file_operations {
	int (*write)(struct file*, const void* buf, int len);
	int (*read)(struct file*, void* buf, int len);
	int (*open)(struct vnode*, struct file**);//open vnode to table
	int (*close)(struct file*);
};

//struct mount* rootfs;

/* From linux vfs.rst It sys.
 *
 */
int register_filesystem(struct filesystem* fs);
int vfs_open(const char* pathname, int flags, struct file **target);
int vfs_close(struct file* file);
int vfs_read(struct file* file , void *buf, size_t len);
int vfs_write(struct file* file , const void* buf, size_t len);

int vfs_mkdir(const char* pathname);
int vfs_create(struct vnode* dir_node, struct vnode** target, const char* name);

int vfs_mount(const char* target, const char* filesystem);
/* To look up an inode need VFS xall `lookup()`of the parent DIR of the 
 * inode. We need to get the Dentry -> Inode. then we can do `open()` ...
 */
int vfs_lookup(const char* pathname, struct vnode** target);


/*
 * FIXME: Don't declare attribute here.  Let each FS do their works.
 *
typedef struct {
	enum FsTy type;
	const char* name;

}FsAttr;
*/

#define O_CREAT 0x0100

// Helper
char* getFileName(char* dest, const char* from);

#endif // VFS_H
