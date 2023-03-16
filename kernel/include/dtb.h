#ifndef DTB_H
#define DTB_H

#include "uart.h"
#include "string.h"
#include "utils.h"

// manipulate device tree with dtb file format
#define FDT_BEGIN_NODE 0x00000001
#define FDT_END_NODE 0x00000002
#define FDT_PROP 0x00000003
#define FDT_NOP 0x00000004
#define FDT_END 0x00000009

typedef void (*dtb_callback)(unsigned int node_type, char *name, void *value, unsigned int name_size);

// stored as big endian
struct fdt_header {
    unsigned int magic;
    unsigned int totalsize;
    unsigned int off_dt_struct;
    unsigned int off_dt_strings;
    unsigned int off_mem_rsvmap;
    unsigned int version;
    unsigned int last_comp_version;
    unsigned int boot_cpuid_phys;
    unsigned int size_dt_strings;
    unsigned int size_dt_struct;
};

void fdt_traverse(dtb_callback callback);
void initramfs_callback(unsigned int node_type, char *name, void *value, unsigned int name_size);

// device variables
extern char *cpio_start;
extern char *cpio_end;


#endif