#include "devtree.h"
#include "string.h"
#include "print.h"

uint32_t u32_to_little_endian(uint32_t num) {
  uint8_t *t = (uint8_t *)&num;
  return (t[3]) | (t[2] << 8) | (t[1] << 16) | (t[0] << 24);
}

void fdt_traverse(void (*initramfs_callback)(const char *, const char *, void *)) {
  printf("DTB_ADDR: %#X\n", DTB_ADDR);

  struct fdt_header *header = (struct fdt_header *)DTB_ADDR;

  if (u32_to_little_endian(header->magic) != DTB_MAGIC) {
    printf("The magic number is not equal to %#X\n", DTB_MAGIC);
    return;
  }

  uint32_t off_struct = u32_to_little_endian(header->off_dt_struct);
  uint32_t off_string = u32_to_little_endian(header->off_dt_strings);

  void *struct_addr = (void *)header + off_struct;
  void *string_addr = (void *)header + off_string;
  char *nodename;

  while (1) {
    uint32_t token = u32_to_little_endian(*((uint32_t *)struct_addr));

    if (token == FDT_BEGIN_NODE) {
      struct_addr += sizeof(uint32_t);
      nodename = struct_addr;
      uint32_t offset = strlen(nodename) + 1; // null-character
      if (offset % 4)
        offset += 4 - offset & 3;
      struct_addr += offset;
    }
    else if (token == FDT_END_NODE) {
      struct_addr += sizeof(uint32_t);
    }
    else if (token == FDT_PROP) {
      struct_addr += sizeof(uint32_t);
      struct fdt_prop *prop = (struct fdt_prop *)struct_addr;
      char *propname = string_addr + u32_to_little_endian(prop->nameoff);
      
      uint32_t len_val = u32_to_little_endian(prop->len);
      if (len_val % 4)
        len_val += 4 - len_val & 3;

      void *prop_val = (void *)prop + sizeof(struct fdt_prop);

      initramfs_callback(nodename, propname, prop_val);

      struct_addr += sizeof(struct fdt_prop) + len_val;
    }
    else if (token == FDT_NOP) {
      struct_addr += sizeof(uint32_t);
    }
    else if (token == FDT_END) {
      struct_addr += sizeof(uint32_t);
      break;
    }
    else {
      print("token not matched\n");
      break; 
    }
  }
  print("Successfully parsed the DeviceTree\n");
}
