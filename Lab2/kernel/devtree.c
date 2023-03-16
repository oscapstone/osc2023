#include "devtree.h"
#include "uart.h"
#include "io.h"

uint32_t fdt32_to_cpu(uint32_t fdt_num) {
  uint8_t *part = (uint8_t*)&fdt_num;
  return (part[0] << 24) | (part[1] << 16) | (part[2] << 8) | (part[3]);
}

uint64_t fdt64_to_cpu(uint64_t fdt_num) {
  uint8_t *part = (uint8_t*)&fdt_num;
  uint64_t val = (part[0] << 24) | (part[1] << 16) | (part[2] << 8) | (part[3]);
  val <<= 32;
  val |= (part[4] << 24) | (part[5] << 16) | (part[6] << 8) | (part[7]);
  return val;
}





static uint32_t fdt_iter_tag(const void *mem, uint32_t off, uint32_t *nextoff) {
  uint32_t *tagp;
  uint32_t tag, len;
  uint32_t offset = off;
  char *node_name;
  uint32_t name_size;
  struct fdt_prop *prop_ptr;

  tagp = (uint32_t*)(mem + off);
  tag = fdt32_to_cpu(*tagp);

  switch (tag) {
  case FDT_BEGIN_NODE:
    node_name = (char*)(mem + off + sizeof(tag));
    for (name_size = 1; node_name[name_size-1] != '\0'; name_size++);
    // print_string("before ");
    // print_num(name_size);
    // print_char('\n');
    // print_num(name_size&3);
    if (name_size & 3){
      name_size += 4 - (name_size & 3); //padding to nearest and bigger than 's 4k
      // print_string("after ");
      // print_num(name_size);
      // print_char('\n');
        
    } 
    offset += sizeof(tag) + name_size;
    // print_num(sizeof(tag));
    // print_char(' ');
    // print_num(name_size);
    // print_char(' ');
    // print_num(offset);
    // print_char('\n');
    
    break;
    
  case FDT_PROP:
    prop_ptr = (struct fdt_prop*)(mem+off+sizeof(tag));
    len = fdt32_to_cpu(prop_ptr->len);
    if (len & 3) len += 4 - (len & 3);
    offset += sizeof(tag) + sizeof(struct fdt_prop) + len;
    break;
    
  case FDT_END_NODE:
  case FDT_NOP:
  case FDT_END:
    offset += sizeof(tag); // nothing left
    break;
  }

  *nextoff = offset;
  return tag;
}


inline static uint32_t fdt_iter_node(const void *mem, uint32_t off, uint32_t *nextoff) {
  uint32_t secoff = off;
  uint32_t tag;
  while (tag != FDT_BEGIN_NODE){
    off = secoff;
    tag = fdt_iter_tag(mem, off, &secoff);
    if (tag == FDT_END) return -1;
  } 
  *nextoff = secoff;
  return off;
}

inline static struct fdt_prop* fdt_iter_prop(const void *mem, uint32_t off, uint32_t *nextoff) {
  uint32_t tag;
  uint32_t secoff = off;
  while (tag != FDT_PROP){
    off = secoff;
    tag = fdt_iter_tag(mem, off, &secoff);
    if (tag == FDT_END_NODE) {
      return 0;
    }
  }
  *nextoff = secoff;
  struct fdt_prop *prop_ptr = (struct fdt_prop*)(mem + off + sizeof(uint32_t));
  return prop_ptr;
}

inline static char* fdt_prop_getname(struct fdt_prop *prop_ptr, void *mem, uint32_t strings_off) {
  return (char*)mem + strings_off + fdt32_to_cpu(prop_ptr->nameoff);
}


void fdt_traverse(void (*func)(const char* node, const char *prop, void* value, uint32_t size)) {
  struct fdt_header *header = (struct fdt_header*)(*DTB_ADDR);
  print_string("DTB_ADDR ");
  print_h((uint32_t)DTB_ADDR);
  print_char('\n');
  print_string("HEADER ");
  print_h((uint32_t)header);
  print_char('\n');

  char *node_name;
  char *prop_name;
  uint32_t struct_off = fdt32_to_cpu(header->off_dt_struct);
  uint32_t strings_off = fdt32_to_cpu(header->off_dt_strings);
  
  
  // print_num(struct_off);
  // print_char('\n');
  // print_num(strings_off);
  // print_char('\n');



  uint32_t node_off;
  uint32_t nextnode_off = struct_off;
  
  print_string("Dtb base addr ");
  
  print_h((uint32_t)(header));
  print_char('\n');

  while (1) {
    node_off = nextnode_off;
    node_off = fdt_iter_node(header, node_off, &nextnode_off);
    if (node_off == -1) break;//end node
    node_name = ((char*)header) + node_off + sizeof(uint32_t);

    
    // print_string("node_name: ");
    // print_string(node_name);
    // print_char('\n');
    // print_string("    ");
    

    
    uint32_t prop_off = node_off;
    struct fdt_prop *prop_ptr;
    while(1) {
      prop_ptr = fdt_iter_prop(header, prop_off, &prop_off);
      if (prop_ptr == 0) { // end node
        break;
      }
      prop_name = fdt_prop_getname(prop_ptr, header, strings_off);
      func(node_name, prop_name,((char*)prop_ptr) + sizeof(struct fdt_prop),fdt32_to_cpu(prop_ptr->len));
      
      // print_string(prop_name);
      // print_string("|");
      
    }
  }
}

