#ifndef _INITRAMFS_H
#define _INITRAMFS_H
extern char *cpio_addr;
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
    int (*exec)(struct initramfs *self, char *argv[]);
};
extern struct initramfs _initramfs;
extern void init_initramfs(struct initramfs *fs);
extern void _cpio_ls(struct initramfs *self, char *path);
extern void _cpio_cat(struct initramfs *self, char *path);
extern int _cpio_exec(struct initramfs *self, char *argv[]);
#endif