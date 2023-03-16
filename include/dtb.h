#ifndef _DTB_H
#define _DTB_H
#include "stdint.h"
extern char *cpio_addr;
typedef uint32_t fdt32_t;
typedef uint64_t fdt64_t;
enum fdt_tag_t {
    FDT_BEGIN_NODE = 0x1,
    FDT_END_NODE = 0x2,
    FDT_PROP = 0x3,
    FDT_NOP = 0x4,
    FDT_END = 0x9
};
typedef struct {
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
    // fdt32_t magic;         // DTB magic number
    // fdt32_t total_size;    // Total size of DTB
    // fdt32_t off_dt_struct; // Offset to structure
    // fdt32_t off_dt_strings;// Offset to strings
    // fdt32_t off_mem_rsvmap;// Offset to memory reserve map
    // fdt32_t version;       // DTB version
    // fdt32_t last_comp_version; // Last compatible version
    // fdt32_t boot_cpuid_phys;// Boot CPU physical ID
    // fdt32_t size_dt_strings;// Size of strings
    // fdt32_t size_dt_struct; // Size of structure
} dtb_header_t;

typedef struct fdt_node_header  {
    fdt32_t tag;
    char name[0];
} dtb_node_t;

typedef struct fdt_property  {
    fdt32_t tag;
    fdt32_t len;
    fdt32_t nameoff;
    char data[0];
} dtb_property_t;

extern struct fdt;
typedef void(*fdt_callback_t)(struct fdt *self, dtb_node_t *node, dtb_property_t *prop, void *data);

struct fdt {
    void *head_addr;
    void *fdt_struct_start;
    void *strings_start;
    uint32_t total_size;
    int (*fdt_traverse)(struct fdt *self, fdt_callback_t cb, void *data);
    int (*fdt_print) (struct fdt *self);
};
extern int _fdt_print(struct fdt *self);
extern int _fdt_traversal(struct fdt *self, fdt_callback_t cb, void *data);
extern struct fdt _fdt;
int fdt_init(struct fdt *fdt, void *addr);
void initramfs_fdt_cb(struct fdt *self, dtb_node_t *node, dtb_property_t *prop, void *data);


#endif