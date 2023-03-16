#ifndef DEVTREE_H
#define DEVTREE_H

#include "stdint.h"

#define DTB_ADDR ((volatile uint64_t *)(0x50000))

struct fdt_header {
  uint32_t magic;
  uint32_t totalsize;
  uint32_t off_dt_struct;
  uint32_t off_dt_strings;
  uint32_t off_mem_rsvmap;
  uint32_t version;
  uint32_t last_comp_version;
  uint32_t boot_cpuid_phys;
  uint32_t size_dt_strings;
  uint32_t size_dt_struct;
};


struct fdt_node_header {
  // fdt32_t tag;
  /* node名称，作为额外数据以'\0'结尾的字符串形式存储在structure block
    * 32-bits对齐，不够的位用0x0补齐
    */
  char name[0];
};

struct fdt_prop {
  // uint32_t tag;
  uint32_t len;
  uint32_t nameoff; 
  /*nameoff gives an offset into the strings block 
  (see Section 5.5) at which 
  the property’s name is stored as a
  null-terminated string*/

};

#define FDT_BEGIN_NODE  0x1             /* Start node: full name */
#define FDT_END_NODE    0x2             /* End node */
#define FDT_PROP        0x3             /* Property: name off,size, content */
#define FDT_NOP         0x4             /* nop */
#define FDT_END         0x9

void fdt_traverse(void (*func)(const char* nd, const char *prop, void* value, uint32_t size));

uint32_t fdt32_to_cpu(uint32_t fdt_num);

uint64_t fdt64_to_cpu(uint64_t fdt_num);

#endif
