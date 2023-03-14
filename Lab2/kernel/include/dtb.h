#ifndef _DTB_H
#define _DTB_H

#ifndef __ASSEMBLY__

struct fdt_header {
    unsigned int magic;          /* magic word FDT_MAGIC */
    unsigned int totalsize;      /* total size of DT block */
    unsigned int off_dt_struct;      /* offset to structure */
    unsigned int off_dt_strings;     /* offset to strings */
    unsigned int off_mem_rsvmap;     /* offset to memory reserve map */
    unsigned int version;        /* format version */
    unsigned int last_comp_version;  /* last compatible version */

    /* version 2 fields below */
    unsigned int boot_cpuid_phys;    /* Which physical CPU id we're
                        booting on */
    /* version 3 fields below */
    unsigned int size_dt_strings;    /* size of the strings block */

    /* version 17 fields below */
    unsigned int size_dt_struct;     /* size of the structure block */
};


struct fdt_node_header {
    unsigned int tag;
    char name[0];
};

struct fdt_property {
    unsigned int tag;
    unsigned int len;
    unsigned int nameoff;
    char data[0];
};

#endif /* !__ASSEMBLY */

#define FDT_MAGIC   0xd00dfeed  /* 4: version, 4: total size */
#define FDT_TAGSIZE sizeof(unsigned int)

#define FDT_BEGIN_NODE  0x1     /* Start node: full name */
#define FDT_END_NODE    0x2     /* End node */
#define FDT_PROP    0x3     /* Property: name off,
                       size, content */
#define FDT_NOP     0x4     /* nop */
#define FDT_END     0x9

#define FDT_V1_SIZE (7*sizeof(unsigned int))
#define FDT_V2_SIZE (FDT_V1_SIZE + sizeof(unsigned int))
#define FDT_V3_SIZE (FDT_V2_SIZE + sizeof(unsigned int))
#define FDT_V16_SIZE    FDT_V3_SIZE
#define FDT_V17_SIZE    (FDT_V16_SIZE + sizeof(unsigned int))
typedef void (*dtb_callback)(unsigned int node_type, char *name, void *value, unsigned int name_size);
void fdt_traverse(dtb_callback callback);
void initramfs_callback(unsigned int node_type, char *name, void *value, unsigned int name_size);
unsigned int endian_big2little(unsigned int x) ;
#endif /* _FDT_H */