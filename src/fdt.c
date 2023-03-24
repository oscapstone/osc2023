#include "fdt.h"
#include "uart.h"
#include "string.h"
unsigned long CPIO_BASE;
unsigned int big2litter_endian(unsigned int num){
  unsigned int ret = 0;
  ret = ret | (num >> 24);
  ret = ret | ((num & 0x00FF0000) >> 8);
  ret = ret | ((num & 0x0000FF00) << 8);
  ret = ret | (num << 24);
  return ret;
}
// void fdt_traverse(fdt_header *header){

void fdt_traverse(fdt_header *header, void (*callback)(fdt_prop*, char*, char*)){
  if(big2litter_endian(header->magic) != FDT_MAGIC) return;
  uint32_t* struct_block_begin = (uint32_t*)((char*)header+big2litter_endian(header->off_dt_struct));
  char* string_block_begin = (char*)header + big2litter_endian(header->off_dt_strings);
  char* node_name;
  uint32_t* struct_block = struct_block_begin;
  while(1){
    // iterate throught the tokens( big endian 32-bit integer)
    uint32_t token = big2litter_endian(*struct_block_begin);
    switch (token){
      case FDT_BEGIN_NODE:
        // FDT_BEGIN_NODE is followed by the node's unit name as extra data 
        node_name = (char*)(struct_block_begin + 1);
        // structure token should aligned in 4 bytes
        unsigned int len_name = 0;
        while(node_name[len_name] != '\0') len_name++;
        len_name += 1;
        // for(len_name = 1; *(node_name + len_name - 1) != '\0'; len_name ++);
        int reminder = len_name % 4;
        // allign 4 bytes
        if(reminder != 0){
          len_name = len_name + (4 - reminder);
        }
        struct_block_begin += len_name/4;
        break;
      case FDT_PROP: ;
        fdt_prop* prop = (fdt_prop*)(struct_block_begin + 1);
        unsigned int len = big2litter_endian(prop->len);
        reminder = len % 4;
        if(reminder != 0){
          len = len + (4 - reminder);
        }
        struct_block_begin += (sizeof(fdt_prop) + len)/4;
        char *propertry_name = string_block_begin + big2litter_endian(prop->nameoff);
        callback(prop, node_name, propertry_name);
        // uart_puts(node_name);
        // uart_puts("-------");
        // uart_puts(propertry_name);
        // uart_puts("\r\n");
        break;
      case FDT_END:
        return;
        break;
      default:
        break;
    }
    struct_block_begin ++;
  }
}

void initramfs_callback(fdt_prop* prop, char* N_name, char*prop_name){
  if(!strcmp(N_name, "chosen") && !strcmp(prop_name, "linux,initrd-start")){
    uint32_t load_addr = *((uint32_t*)(prop + 1));
    CPIO_BASE = big2litter_endian(load_addr);
    // uart_puts("CPIO_BASE: ");
    // uart_hex(CPIO_BASE);
    // uart_puts("\n\r");
  }
}

