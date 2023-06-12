#ifndef VFS_H
#define VFS_H

#define size_t int

struct vnode
{
	struct mount* mount;
	struct vnode_operations* v_ops;
	struct file_operations* f_ops;
	void* internal;
	int file_size;
	char name[16];
	const char* node_type;
	struct vnode* parent;
	struct vnode** child;	//will be struct vnode* [20]
	int child_num;
};

struct file
{
	struct vnode* vnode;
	size_t f_pos;
	struct file_operations* f_ops;
	int flags;
};

struct mount
{
	struct vnode* root;
	struct filesystem* fs;
};

struct filesystem
{
	const char* name;
	int (*setup_mount)(struct filesystem* fs,struct mount* mount);
	struct filesystem* next;
};

struct file_operations
{
	int (*write)(struct file* file,const void* buf,size_t len);
	int (*read)(struct file* file,void* buf,size_t len);
	int (*open)(struct vnode* file_node,struct file** target);
	int (*close)(struct file* file);
	long (*lseek64)(struct file* file,long offset,int whence);
};

struct vnode_operations
{
	int (*lookup)(struct vnode* dir_node,struct vnode** target,const char* component_name);
	int (*create)(struct vnode* dir_node,struct vnode** target,const char* component_name);
	int (*mkdir)(struct vnode* dir_node,struct vnode** target,const char* component_name);
};

void rootfs_init();
int register_filesystem(struct filesystem* fs);
int vfs_lookup(const char* pathname, struct vnode** target);
int vfs_open(const char* pathname, int flags, struct file** target);
int vfs_close(struct file* file);
int vfs_write(struct file* file, const void* buf, size_t len);
int vfs_read(struct file* file, void* buf, size_t len);
int vfs_mkdir(const char* pathname);
int vfs_mount(const char* target, const char* filesystem);

#endif
