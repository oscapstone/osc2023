
#ifndef __DEVICETREE__
#define __DEVICETREE__
#include <stdint.h>
#include <stddef.h>

#define FDT_BEGIN_NODE  0x00000001
#define FDT_END_NODE    0x00000002
#define FDT_PROP        0x00000003
#define FDT_NOP         0x00000004
#define FDT_END         0x00000009

extern void *_devicetree_begin;

typedef struct _fdt_header { // 5.2 big-endian
    uint32_t magic;             // 0xd00dfeed
    uint32_t totalsize;         // 
    uint32_t off_dt_struct;     // offset in bytes of the devicetree struct
    uint32_t off_dt_strings;    // offset in bytes of the strings block
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;   // the length in bytes of the strings block
    uint32_t size_dt_struct;    // the length in bytes of the structure block
} fdt_header;

typedef struct _fdt_prop{ // 5.4 FDT_PROP (0x00000003) 
    uint32_t len;
    //uint32_t nameoff;
    const char *name;
    const char *value;
} fdt_prop;

typedef int (*initramfs_callback)(char* node, const char *name, void *data);
void fdt_traverse(initramfs_callback f, void *data);
char *fdt_nextnode(char *fdt_addr, char *fdt_struct_end);
fdt_prop *fdt_nextprop(char *fdt_addr, char **nexttok);
#endif