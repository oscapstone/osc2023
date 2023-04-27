#include "type.h"

#ifndef __CPIO_H

#define __CPIO_H


// char *_cpio_buf = (char *)INITRAMFS_ADDR;

typedef struct Cpio_newc_header {
    char   c_magic[6];
    char   c_ino[8];
    char   c_mode[8];
    char   c_uid[8];
    char   c_gid[8];
    char   c_nlink[8];
    char   c_mtime[8];
    char   c_filesize[8];
    char   c_devmajor[8];
    char   c_devminor[8];
    char   c_rdevmajor[8];
    char   c_rdevminor[8];
    char   c_namesize[8];
    char   c_check[8];
}__attribute__((packed)) cpio_newc_header;

// unsigned int next_file(cpio_newc_header *header, char** buf);
void list_files();
unsigned int cat_file(const char *file);
void set_initramfs_addr(uint32_t addr);
unsigned int load_program(const char *file);

#endif