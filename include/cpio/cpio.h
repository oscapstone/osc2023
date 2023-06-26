#ifndef __CPIO_H
#define __CPIO_H
#include "type.h"
#include "fs/vfs.h"


#define C_FMASK 0170000
#define C_SOCK 0140000  // File type	value for sockets.
#define C_SYMBOL 0120000 // File type	value for symbolic links.
#define C_REG 0100000  // File type	value for regular files.
#define C_BLKDEV 0060000  // File type	value for block	special	devices.
#define C_DIR 0040000  // File type	value for directories.
#define C_CHDEV 0020000  // File type	value for character special devices.
#define C_FIFO 0010000  // File type	value for named	pipes or FIFOs.

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
uint8_t get_file(const char *filename, char **content, unsigned int *c_filesize);
void mount_initramfs();
int init_cpio_fs(struct filesystem *fs);

#endif