#ifndef FDT_H

#define FDT_H
#include <stdint.h>
//header
typedef struct
{
    uint32_t magic;                 //value 0xD00DFEED (big endian)
    uint32_t totalsize;             //total_size
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;               //version 17
    uint32_t last_comp_version;     //should contain lowest version of the device tree structure
    uint32_t boot_cpuid_phys;       //contain physical ID of the boot CPU
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
}fdt_header;

//Memory Reservation Block (aligned to 8byte)
typedef struct                      //provide the client program an area in physical memory
{
    uint64_t address;               //terminated with entry both address & size are equal to zero
    uint64_t size;
}fdt_reserve_entry;

//Structure Block (aligned to 4byte)
#define FDT_BEGIN_NODE  0x00000001  //begin of the node
#define FDT_END_NODE    0x00000002  //end of the node
#define FDT_PROP        0x00000003  //one property in device tree
#define FDT_NOP         0x00000004  //will be ignored by parser
#define FDT_END         0x00000009  //end of the structure
typedef struct
{
    uint32_t len;                   //length of th property's value in byte
    uint32_t nameoff;               //give the offset into string block (name is stored as a null_terminated string)
}fdt_prop;

//String Block (no align constraint)

uint32_t to_little_endian_32(uint32_t data);
uint64_t to_little_endian_64(uint64_t data);
void to_little_endian_header(fdt_header* fdt);
void fdt_header_info(fdt_header* fdt);
void memresv_block_info(char* fdt,int offset);
void struct_block_info(char* fdt,int struct_offset,int string_offset);
void string_block_info(char* fdt,int offset);
void fdt_info(char* fdt_origin);
void fdt_api(char* fdt_origin,void(*func)(char* value),char* keyword);
#endif
