#ifndef _INITRAMFS_H
#define _INITRAMFS_H
#include "stdint.h"
#include "utils.h"
#include "vfs.h"
// #define INIT_KSIZE 0x4000
extern char *cpio_addr, *cpio_end;
//struct __attribute__((__packed__)) cpio_newc_header {
struct cpio_newc_header {
    char	   c_magic[6];
    char	   c_ino[8];
    char	   c_mode[8];
    char	   c_uid[8];
    char	   c_gid[8];
    char	   c_nlink[8];
    char	   c_mtime[8];
    char	   c_filesize[8];
    char	   c_devmajor[8];
    char	   c_devminor[8];
    char	   c_rdevmajor[8];
    char	   c_rdevminor[8];
    char	   c_namesize[8];
    char	   c_check[8];
};

struct initramfs {
    char *addr;
    void (*ls)(struct initramfs *self, char *path);
    void (*cat)(struct initramfs *self, char *path);
    char *(*file_content)(struct initramfs *self, char *path, size_t *fsize);
    int (*exec)(struct initramfs *self, char *argv[]);
};
extern struct initramfs _initramfs;
extern void init_initramfs(struct initramfs *fs);

// make initramfs as an read only file system which follows the VFS interface
extern struct file_operations initramfs_f_ops;
extern struct vnode_operations initramfs_v_ops;
extern int initramfs_setup_mount(struct filesystem* fs, struct mount* mount);
extern int initramfs_write(struct file* file, const void* buf, size_t len);
extern int initramfs_read(struct file* file, void* buf, size_t len);
extern int initramfs_open(struct vnode* file_node, struct file** target);
extern int initramfs_close(struct file* file);
extern long initramfs_lseek64(struct file* file, long offset, int whence);

extern int initramfs_lookup(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
extern int initramfs_create(struct vnode* dir_node, struct vnode** target,
            const char* component_name);
extern int initramfs_mkdir(struct vnode* dir_node, struct vnode** target,
            const char* component_name);
#endif