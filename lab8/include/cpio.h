#ifndef __CPIO_H__
#define __CPIO_H__

/*
Each file system object in	a cpio archive comprises a header record with
basic numeric metadata followed by	the full pathname of the entry and the
file data.	 The header record stores a series of integer values that gen-
erally follow the fields in struct	stat.  (See stat(2) for	details.)  The
variants differ primarily in how they store those integers	(binary, oc-
tal, or hexadecimal).  The	header is followed by the pathname of the en-
try (the length of	the pathname is	stored in the header) and any file
data.  The	end of the archive is indicated	by a special record with the
pathname "TRAILER!!!"
*/

#define CPIO_HEADER_MAGIC       "070701"
#define CPIO_FOOTER_MAGIC       "TRAILER!!!"
#define PI_CPIO_BASE            ((void*) (0x20000000))
#define QEMU_CPIO_BASE          ((void*) (0x8000000))

#include "devtree.h"

extern void *DEVTREE_CPIO_BASE;

struct cpio_newc_header {
    char	   c_magic[6]; // 070701
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
    char	   c_check[8]; // ignored by readers
};

void initramfs_callback (char *, char *, struct fdt_prop *);
void cpio_ls ();
void cpio_cat ();
void cpio_exec ();

unsigned int hexstr_to_uint(char *s, unsigned int len);

#endif