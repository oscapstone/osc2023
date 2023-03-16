#ifndef _CPIO_H
#define _CPIO_H

#include "type.h"

struct cpio_newc_header {
  char    c_magic[6];   /* 6   */
  char    c_ino[8];     /* 14  */
  char    c_mode[8];    /* 22  */
  char    c_uid[8];     /* 30  */
  char    c_gid[8];     /* 38  */
  char    c_nlink[8];   /* 46  */
  char    c_mtime[8];   /* 54  */
  char    c_filesize[8];  /* 62  */
  char    c_devmajor[8];  /* 70  */
  char    c_devminor[8];  /* 78  */
  char    c_rdevmajor[8]; /* 86  */
  char    c_rdevminor[8]; /* 94  */
  char    c_namesize[8];  /* 102 */
  char    c_check[8];     /* 110 */
}; /* 110 */

typedef struct cpio_newc_header cpio_newc_header_t;

static void *INITRD_ADDR = 0;

void initramfs_callback(const char *, const char *, void *);

void cpio_ls();
void cpio_cat(char *, int);

#endif
