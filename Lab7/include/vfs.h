#ifndef _VFS_H
#define _VFS_H

#include <stddef.h>

#define O_CREAT 00000100
#define DIR 0
#define FILE 1
#define COMPONENT_NAME_MAX 32 // include '\0'
#define MAX_NUM_OF_ENTRY 16
#define TMPFS_FILE_SIZE_MAX 4096
#define PATHNAME_MAX 256 // include '\0'
#define VFS_PROCESS_MAX_OPEN_FILE 16

typedef struct vnode
{
    struct mount *mount;
    struct vnode_operations *v_ops;
    struct file_operations *f_ops;
    struct node_info *internal;
} vnode_t;

typedef struct node_info
{
    char *name;
    unsigned int type;
    unsigned int size; // if type is 'DIR' means # of entries, if is 'FILE' means size of file(in bytes).
    vnode_t **entry;
    char *data;
} node_info_t;

// file handle
typedef struct file
{
    struct vnode *vnode;
    size_t f_pos; // RW position of this file handle
    struct file_operations *f_ops;
    int flags;
} file_t;

typedef struct mount
{
    struct vnode *root;
    struct filesystem *fs;
} mount_t;

typedef struct filesystem
{
    char *name;
    int (*setup_mount)(struct filesystem *fs, struct mount *mount);
} filesystem_t;

typedef struct vnode_operations
{
    int (*lookup)(struct vnode *dir_node, struct vnode **target, char *component_name);
    int (*create)(struct vnode *dir_node, struct vnode **target, char *component_name);
    int (*mkdir)(struct vnode *dir_node, struct vnode **target, char *component_name);
} vnode_operations_t;

typedef struct file_operations
{
    int (*write)(struct file *file, void *buf, size_t len);
    int (*read)(struct file *file, void *buf, size_t len);
    int (*open)(struct vnode *file_node, struct file **target);
    int (*close)(struct file *file);
    long (*lseek64)(struct file *file, long offset, int whence);
} file_operations_t;

extern struct mount *rootfs;
extern char cwdpath[256];
extern file_t *kfd[16];
extern int fd_count;

int register_filesystem(struct filesystem *fs);
int vfs_mount(char *target, char *filesystem);
int vfs_lookup(char *pathname, struct vnode **target);
int vfs_create(char *pathname);
int vfs_mkdir(char *pathname);

int vfs_open(char *pathname, int flags, struct file **target);
int vfs_close(struct file *file);
int vfs_write(struct file *file, void *buf, size_t len);
int vfs_read(struct file *file, void *buf, size_t len);

void get_next_component(char *pathname, char *target, int level);
void basename(char *src, char *des);
void dirname(char *src, char *des);
void handle_path(char *rela, char *abso);

void vfs_ls(char *pathname, int flag);
int vfs_cd(char *target_dir);
long vfs_lseek(struct file *file, long offset, int whence);
long vfs_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

#endif