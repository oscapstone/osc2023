#ifndef __DTB_H
#define __DTB_H

#include "type.h"
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

struct fdt_prop {
    uint32_t len;
    uint32_t nameoff;
};


typedef void (*initramfs_callback_func)(char *name, char *prop_name, char *data);
void fdt_param_init(void *ptr);
void fdt_traverse(initramfs_callback_func f);


#endif