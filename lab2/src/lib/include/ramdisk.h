#ifndef __RAMDISK__
#define __RAMDISK__
#include <stdint.h>
#include <stddef.h>

struct cpio_newc_header {
    char c_magic[6];    // 070702
    char c_ino[8];      // inode numbers
    char c_mode[8];     // mode
    char c_uid[8];      // uid
    char c_gid[8];      // gid
    char c_nlink[8];    // link number
    char c_mtime[8];
    char c_filesize[8]; // content size
    char c_devmajor[8];
    char c_devminor[8];
    char c_rdevmajor[8];
    char c_rdevminor[8];
    char c_namesize[8]; // file name length
    char c_check[8];    // sum of all bytes
};

typedef struct _cpio_file{
    char *header;
    uint32_t ino;
    uint32_t mode;
    uint32_t uid;
    uint32_t gid;
    uint32_t nlink;
    uint32_t mtime;
    uint32_t filesize;
    uint32_t devmajor;
    uint32_t devminor;
    uint32_t rdevmajor;
    uint32_t rdevminor;
    uint32_t namesize;
    uint32_t check;
    char *name;
    char *content;
    struct _cpio_file *next;
} cpio_file;

int ramdisk_get_addr();
cpio_file* cpio_parse();

#endif