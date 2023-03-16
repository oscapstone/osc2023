#include "cpio.h"
#include "string.h"
#include "io.h"

static uint32_t stoul(char *s, uint32_t len) {
  uint32_t val = 0;
  // print_num(len);
  for (int slen = 0; slen < len && s[slen] != 0; slen++) { //8
    val <<= 4;
    if (s[slen] >= 'A' && s[slen] <= 'F') {
      val += s[slen] - 'A' + 10;
    } else if (s[slen] >= 'a' && s[slen] <= 'f') {
      val += s[slen] - 'a' + 10;
    } else if (s[slen] >= '0' && s[slen] <= '9') {
      val += s[slen] - '0';
    } else {
      return -1;
    }
  }
  
  return val;
}

inline static uint32_t cpio_filenamesize(struct cpio_newc_header *p_header) {
  // print_string("5555");
  return stoul(p_header->c_namesize, sizeof(p_header->c_namesize));
}

uint32_t cpio_filename(struct cpio_newc_header *p_header, char *name_buf,uint32_t size) {
  
  uint64_t nsize = cpio_filenamesize(p_header);
  // print_string("333");
  char *name = ((char*)p_header) + sizeof(struct cpio_newc_header);
  int i;
  for (i = 0; i < nsize && i < size; i++) {
    name_buf[i] = name[i];
  }
  name_buf[i+1] = ( i < size) ? 0 : name_buf[i+1];
  
  return nsize;
}

uint32_t cpio_filesize(struct cpio_newc_header *p_header) {
  return stoul(p_header->c_filesize, sizeof(p_header->c_filesize));
}

uint32_t cpio_read(struct cpio_newc_header *p_header, uint32_t offset,
                   char *buf, uint32_t size) {
  uint32_t nsize = cpio_filenamesize(p_header);
  if ((nsize + sizeof(struct cpio_newc_header)) & 3) {
    nsize += 4 - ((nsize + sizeof(struct cpio_newc_header)) & 3);
  }
  uint32_t fsize = cpio_filesize(p_header) - offset;
  char *content = ((char*)p_header) + sizeof(struct cpio_newc_header) + nsize + offset;
  int i;
  for (i = 0; i < fsize && i < size; i++) {
    buf[i] = content[i];
  }
  
  return i;
}

struct cpio_newc_header *cpio_nextfile(struct cpio_newc_header* p_header) {
  uint32_t nsize = cpio_filenamesize(p_header);
  uint32_t fsize = cpio_filesize(p_header);

  if ((nsize + sizeof(struct cpio_newc_header)) & 3) {
    nsize += 4 - ((nsize + sizeof(struct cpio_newc_header)) & 3);
  }
  
  if (fsize & 3) {
    fsize += 4 - (fsize & 3);
  }
  
  struct cpio_newc_header* p_next = (struct cpio_newc_header*)
    (((char*)p_header)+sizeof(struct cpio_newc_header)+nsize+fsize);
  
  char fname[sizeof("TRAILER!!!")];
  cpio_filename(p_next, fname, sizeof(fname));
  
  if (strcmp(fname, "TRAILER!!!")) {
    return (struct cpio_newc_header*)0;
  }
  
  return p_next;
}

struct cpio_newc_header *cpio_first(uint64_t addr) {
  struct cpio_newc_header* p_header = (struct cpio_newc_header*)addr;
  // print_string("1111");
  char fname[sizeof("TRAILER!!!")];
  // print_string("2222");
  cpio_filename(p_header, fname, sizeof(fname));
  if (strcmp(fname, "TRAILER!!!")) {
    return (struct cpio_newc_header*)0;
  }
  return p_header;
}


