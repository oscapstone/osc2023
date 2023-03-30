#ifndef _DEVTREE_H
#define _DEVTREE_H

/*
    magic; 0xd00dfeed (big-endian)
    
    totalsize; total size in bytes of the devicetree 
    data structure
    
    off_dt_struct; offset in bytes of the structure block 
    from the beginning of the header
    
    off_dt_strings; offset in bytes of the strings block 
    from the beginning of the header
    
    off_mem_rsvmap; offset in bytes of the memory reservation 
    block from the beginning of the header
    
    version; version of the devicetree data structure
    
    last_comp_version; lowest version of the devicetree data 
    structure with which the version used is backwards compatible
    
    boot_cpuid_phys; physical ID of the system’s boot CPU
    
    size_dt_strings; length in bytes of the strings block 
    section of the devicetree blob
    
    size_dt_struct; length in bytes of the structure block 
    section of the devicetree blob
*/

#define FDT_HEADER_MAGIC    0xd00dfeed
#define FDT_BEGIN_NODE      0x00000001
#define FDT_END_NODE        0x00000002
#define FDT_PROP            0x00000003
#define FDT_NOP             0x00000004
#define FDT_END             0x00000009

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

struct fdt_prop {
    unsigned int len;
    unsigned int nameoff;
};

void devtree_getaddr ();
void fdt_traverse ( void (*callback)(char *, char *, struct fdt_prop *) );

// ARM uses little endian
unsigned int to_lendian (unsigned int);

#endif