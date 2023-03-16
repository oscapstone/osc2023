#ifndef __CPIO_H__
#define __CPIO_H__

#include "devicetree.h"

#define CPIO_MAGIC      "070701"
#define CPIO_FOOTER     "TRAILER!!!"

#define CPIO_QEMU_BASE  0x8000000
#define CPIO_RPI_BASE   0x20000000

struct cpio_newc_header {
    char c_magic[6];        // The string "070701"
    char c_ino[8];
    char c_mode[8];
    char c_uid[8];
    char c_gid[8];
    char c_nlink[8];
    char c_mtime[8];
    char c_filesize[8];
    char c_devmajor[8];
    char c_devminor[8];
    char c_rdevmajor[8];
    char c_rdevminor[8];
    char c_namesize[8];
    char c_check[8];        // This field is always set to zero by writers and ignored by readers.
};

void cpio_list(void);
void cpio_concatenate(void);
void initramfs_callback(char *nodename, char *propname, struct fdt_prop* prop);

#endif

/*New ASCII Format
     The pathname is followed by NUL bytes so that the total size of the fixed
     header plus pathname is a multiple	of four.  Likewise, the	file data is
     padded to a multiple of four bytes.  Note that this format	supports only
     4 gigabyte	files 
     In	this format, hardlinked	files are handled by setting the filesize to
     zero for each entry except	the first one that appears in the archive.
     */