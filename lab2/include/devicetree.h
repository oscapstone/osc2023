#ifndef __DEVICETREE_H__
#define __DEVICETREE_H__

#define FDT_HEADER_MAGIC    0xd00dfeed
#define FDT_BEGIN_NODE      0x00000001
#define FDT_END_NODE        0x00000002
#define FDT_PROP            0x00000003
#define FDT_NOP             0x00000004
#define FDT_END             0x00000009

struct fdt_header {
    unsigned int magic;             // the value 0xd00dfeed (big-endian)
    unsigned int totalsize;         // the total size in bytes of the devicetree
    unsigned int off_dt_struct;     // the offset in bytes of the structure block from the beginning of the header
    unsigned int off_dt_strings;    // the offset in bytes of the strings block from the beginning of the header
    unsigned int off_mem_rsvmap;    // the offset in bytes of the memory reservation block from the beginning of the header
    unsigned int version;           // the version of the devicetree data structure
    unsigned int last_comp_version; // the lowest version of the devicetree data structure with which the version used in backwards compatible
    unsigned int boot_cpuid_phys;   // the physical ID of the system's boot CPU
    unsigned int size_dt_strings;   // the length in bytes of the strings block section of the devicetree blob
    unsigned int size_dt_struct;    // the length in bytes of the structure block section of the devicetree blob
};

struct fdt_prop {
    unsigned int len;               // the length of the property's value in bytes (big-endian)
    unsigned int nameoff;           // an offset into the strings block at which the property's name is stored as a null-terminated string (big-endian)
};

unsigned int to_little_endian(unsigned int num);
void devicetree_get_address(void);
void fdt_traverse(void (*callback)(char*, char*, struct fdt_prop*));

#endif