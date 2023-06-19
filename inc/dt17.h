// Spec: https://github.com/devicetree-org/devicetree-specification/releases/tag/v0.4-rc1
#ifndef _DT_H
#define _DT_H

#include <type.h>

typedef int (*fdt_parser)(int level, char *cur, char *dt_string);

void parse_dtb(char *fdt, fdt_parser parser);
int initramfs_parse_fdt(int level, char *cur, char *dt_strings);
void initramfs_init(char *fdt_base);
void dtb_traverse(char *fdt_base);

void *_initramfs_addr;
void *_initramfs_end;

#define FDT_MAGIC_NUM  0xd00dfeed
#define FDT_BEGIN_NODE 0x00000001
#define FDT_END_NODE   0x00000002
#define FDT_PROP       0x00000003
#define FDT_NOP        0x00000004
#define FDT_END        0x00000009

struct fdt_header {
    fdt32_t magic;               //0xd00dfeed
    fdt32_t totalsize;           //total size in bytes of the dt data structure
    fdt32_t off_dt_struct;       //offset in bytes of the structure block
    fdt32_t off_dt_strings;      //offset in bytes of the strings block
    fdt32_t off_mem_rsvmap;      //offset in bytes of the memory reservation block
    fdt32_t version;             //version of device tree datastructure
    fdt32_t last_comp_version;   //the lowest version of the devicetree data structure is compatible
    fdt32_t boot_cpuid_phys;     //physical ID of the system's boot CPU 
    fdt32_t size_dt_strings;     //length in bytes of the strings block section of the device tree
    fdt32_t size_dt_struct;      //length in bytes of the structure block section of the device tree blob
};

struct fdt_reverse_entry{
    fdt64_t address;
    fdt64_t size;
};

struct fdt_property{
    fdt32_t len;
    fdt32_t nameoff;
};


// Reference: https://elixir.bootlin.com/linux/v5.16.14/source/scripts/dtc/libfdt/libfdt.h#L249
#define fdt_get_header(fdt, field) \
	(fdt32_ld(&((const struct fdt_header *)(fdt))->field))
#define fdt_magic(fdt)			(fdt_get_header(fdt, magic))
#define fdt_totalsize(fdt)		(fdt_get_header(fdt, totalsize))
#define fdt_off_dt_struct(fdt)		(fdt_get_header(fdt, off_dt_struct))
#define fdt_off_dt_strings(fdt)		(fdt_get_header(fdt, off_dt_strings))
#define fdt_off_mem_rsvmap(fdt)		(fdt_get_header(fdt, off_mem_rsvmap))
#define fdt_version(fdt)		(fdt_get_header(fdt, version))
#define fdt_last_comp_version(fdt)	(fdt_get_header(fdt, last_comp_version))
#define fdt_boot_cpuid_phys(fdt)	(fdt_get_header(fdt, boot_cpuid_phys))
#define fdt_size_dt_strings(fdt)	(fdt_get_header(fdt, size_dt_strings))
#define fdt_size_dt_struct(fdt)		(fdt_get_header(fdt, size_dt_struct))

#define fdtp_get_header(fdtp, field) \
    (fdt32_ld(&((const struct fdt_property *)(fdtp))->field))
#define fdtp_len(fdtp)           (fdtp_get_header(fdtp, len))
#define fdtp_nameoff(fdtp)       (fdtp_get_header(fdtp, nameoff))

// little endian to big endian
static inline fdt32_t fdt32_ld(const fdt32_t *p)
{
	const uint8_t *bp = (const uint8_t *)p;

	return ((fdt32_t)bp[0] << 24)
		| ((fdt32_t)bp[1] << 16)
		| ((fdt32_t)bp[2] << 8)
		| bp[3];
}

#endif