#ifndef _INITRAMFS_H
#define _INITRAMFS_H
#include "stdint.h"
#include "utils.h"
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
#endif