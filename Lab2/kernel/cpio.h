#ifndef CPIO_H
#define CPIO_H

#include "stdint.h"

struct cpio_newc_header {
  char c_magic[6];
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
  char c_check[8];
};

uint32_t cpio_filename(struct cpio_newc_header *p_header, char *buf,
                       uint32_t size);
uint32_t cpio_filesize(struct cpio_newc_header *p_header);
uint32_t cpio_read(struct cpio_newc_header *p_header, uint32_t offset,
                   char *buf, uint32_t size);
struct cpio_newc_header *cpio_nextfile(struct cpio_newc_header *);
struct cpio_newc_header *cpio_first(uint64_t addr);

#endif

