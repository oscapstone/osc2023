#ifndef __DTB__
#define __DTB__
#include "uart.h"
#include "string.h"

#define PADDING         0x00000000
#define FDT_BEGIN_NODE  0x00000001
#define FDT_END_NODE    0x00000002
#define FDT_PROP        0x00000003
#define FDT_NOP         0x00000004
#define FDT_END         0x00000009


#define DT_ADDR         0x200000




typedef void (*dtb_callback)(uint32_t node_type, char *name, void *value, uint32_t name_size);

uint32_t uint32_endian_big2lttle(uint32_t data);
void traverse_device_tree(void *base,dtb_callback callback);  //traverse dtb tree
void dtb_callback_show_tree(uint32_t node_type, char *name, void *value, uint32_t name_size);
void dtb_callback_initramfs(uint32_t node_type, char *name, void *value, uint32_t name_size);

/* All the header fields are 32-bit integers, stored in big-endian format. */
struct fdt_header {
    uint32_t magic;                 /* The value 0xd00dfeed (big-endian).*/
    uint32_t totalsize;             /* The total size in bytes of the devicetree data structure. */
    uint32_t off_dt_struct;         /* The offset in bytes of the structure block from the beginning of the header.*/
    uint32_t off_dt_strings;        /* The offset in bytes of the strings block from the beginning of the header.*/
    uint32_t off_mem_rsvmap;        /* The offset in bytes of the memory reservation block from the beginning of the header.*/
    uint32_t version;               /* The version of the devicetree data structure.*/
    uint32_t last_comp_version;     /* The lowest version of the devicetree data structure with which the version used is backwards compatible*/
    uint32_t boot_cpuid_phys;       /* The physical ID of the system’s boot CPU.*/
    uint32_t size_dt_strings;       /* The length in bytes of the strings block section of the devicetree blob.*/
    uint32_t size_dt_struct;        /* The length in bytes of the structure block section of the devicetree blob.*/
};

#endif
