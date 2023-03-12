#include "cpio.h"

#include "string.h"
#include "print.h"

#define CPIO_HEADER_MAGIC "070701"

inline uint32_t get_file_size(cpio_newc_header_t *cpio_header) {
  return strtoui(cpio_header->c_filesize, 8, 16);
}

inline uint32_t get_name_size(cpio_newc_header_t *cpio_header) {
  return strtoui(cpio_header->c_namesize, 8, 16);
}

void read_filename(char *cpio_addr, char *filename, int len) {
  char *fname_addr = cpio_addr + sizeof(cpio_newc_header_t);
  int i;
  for (i = 0; i < len; i++) {
    filename[i] = fname_addr[i];
  }
  filename[i] = '\0';
}

char *cpio_list_one(char *addr, char *filename) {
  char *addrptr = addr;
  cpio_newc_header_t *file_header = (cpio_newc_header_t *) addr;

  if (strneq(file_header->c_magic, CPIO_HEADER_MAGIC, 6) != 0) {
    return (char *)0;
  }

  uint32_t filename_size = get_name_size(file_header);
  uint32_t content_size = get_file_size(file_header);

  read_filename(addr, filename, filename_size);
  if (streq(filename, "TRAILER!!!") == 0) {
    return (char *)0;
  }

  if ((filename_size + sizeof(cpio_newc_header_t)) % 4 != 0) {
    filename_size += 4 - ((filename_size + sizeof(cpio_newc_header_t)) & 3);
  }

  if (content_size % 4 != 0) {
    content_size += 4 - (content_size & 3);
  }

  addrptr += sizeof(cpio_newc_header_t) + filename_size + content_size;
  return addrptr;
}

void cpio_ls(char *addr) {
  while (1) {
    char filename[sizeof("TRAILER!!!")];
    addr = cpio_list_one(addr, filename);
    if (addr == (char *)0)
      break;
    printf("%s\n", filename);
  }
}

void cpio_cat(char *command, int len, char *addr) {
  char *new_addr;
  while (1) {
    char filename[sizeof("TRAILER!!!")];
    new_addr = cpio_list_one(addr, filename);
    if (new_addr == (char *)0)
      break;

    if (strneq(command, filename, len) == 0) {
      cpio_newc_header_t *file_header = (cpio_newc_header_t *)addr;
      uint32_t filename_size = get_name_size(file_header);
      uint32_t content_size = get_file_size(file_header);

      if ((filename_size + sizeof(cpio_newc_header_t)) % 4 != 0) {
        filename_size += 4 - ((filename_size + sizeof(cpio_newc_header_t)) & 3);
      }
      if (content_size % 4 != 0) {
        content_size += 4 - (content_size & 3);
      }

      addr += sizeof(cpio_newc_header_t) + filename_size;
      for (int i = 0; i < content_size; i++) {
        printf("%c", addr[i]);
      }
      print("\n");
    }
    addr = new_addr;
  }
}
