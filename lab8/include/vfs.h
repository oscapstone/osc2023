#ifndef _VFS_H
#define _VFS_H

struct vnode {
    struct vnode* parent;
    struct mount* mount;
    struct vnode_operations* v_ops;
    struct file_operations* f_ops;
    void* internal;
};

// file handle
struct file {
    struct vnode* vnode;
    unsigned f_pos;  // RW position of this file handle
    struct file_operations* f_ops;
    int flags;
};

struct mount {
    struct vnode* root;
    struct filesystem* fs;
};

struct filesystem {
    char* name;
    int (*setup_mount)(struct filesystem* fs, struct mount* mount);
};

struct file_operations {
    int (*write)(struct file* file, const void* buf, unsigned len);
    int (*read)(struct file* file, void* buf, unsigned len);
    int (*open)(struct vnode* file_node, struct file** target);
    int (*close)(struct file* file);
};

struct vnode_operations {
    int (*lookup)(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
    int (*create)(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
    int (*mkdir)(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
    int (*load_vnode)(struct vnode* dir_node, char *component_name);
};

struct fd_table {
    int count;
    struct file *fds[16];
};

#define VFS_PATHMAX     256

#define REGULAR_FILE    -2
#define DIRECTORY       -3

#define O_CREAT         00000100
#define SUCCESS         0
#define FAIL            -1
#define EOF             -1

extern struct mount *rootfs;

void rootfs_init();
void initramfs_init();
void fat32_init();

int register_fs(struct filesystem *fs);
struct file* create_fd(struct vnode* target, int flags);
int vfs_open(const char *pathname, int flags, struct file **target);
int vfs_close(struct file *file);
int vfs_write(struct file *file, const void *buf, unsigned len);
int vfs_read(struct file *file, void *buf, unsigned len);

int vfs_mkdir(const char *pathname);
int vfs_mount(const char *target, const char *filesystem);
int vfs_lookup(const char *pathname, struct vnode **target);
int vfs_chdir(const char *pathname);

void traverse(const char* pathname, struct vnode** target_node, char *target_path);
void r_traverse(struct vnode *node, const char *path, struct vnode **target_node, char *target_path);

#endif