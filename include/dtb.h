#ifndef DTB_H
#define DTB_H

#include "my_stdint.h"

#define FTB_MAGIC ((unsigned int) 0xd00dfeed)

typedef void (*device_node_callback_t)(const void *node);

typedef struct _fdt_header {
    uint32_t magic; // magic number should be FTB_MAGIC
    uint32_t totalsize; // total size of dt block
    uint32_t off_dt_struct; // offset to string
    uint32_t off_dt_strings; // offset to structure
    uint32_t off_mem_rsvmap; // offset to memory reserve map
    uint32_t version; // format version
    uint32_t last_comp_version; // last compatible version
    uint32_t boot_cpuid_phys; // which cpu we are boot on
    uint32_t size_dt_strings; // size of the strings block
    uint32_t size_dt_struct; // size of the structure block
} fdt_header;

extern fdt_header *dtb_address;

void fdt_init();

// void fdt_traverse(device_node_callback_t callback);
void fdt_traverse();


#endif