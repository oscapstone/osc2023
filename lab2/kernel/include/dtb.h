#include <stdint.h>
#ifndef DTB_H
#define DTB_H
/*
 * libfdt - Flat Device Tree manipulation
 * Copyright (C) 2006 David Gibson, IBM Corporation.
 * Copyright 2012 Kim Phillips, Freescale Semiconductor.
 */

typedef union
{
	uint32_t u32;
	unsigned char u8[4];
} u_32;

typedef union
{
	uint64_t u64;
	unsigned char u8[8];
} u_64;

typedef struct
{
	uint32_t magic;				/* magic word FDT_MAGIC */
	uint32_t totalsize;			/* total size of DT block */
	uint32_t off_dt_struct;		/* offset to structure */
	uint32_t off_dt_strings;	/* offset to strings */
	uint32_t off_mem_rsvmap;	/* offset to memory reserve map */
	uint32_t version;			/* format version */
	uint32_t last_comp_version; /* last compatible version */

	/* version 2 fields below */
	uint32_t boot_cpuid_phys; /* Which physical CPU id we're
					 booting on */
	/* version 3 fields below */
	uint32_t size_dt_strings; /* size of the strings block */

	/* version 17 fields below */
	uint32_t size_dt_struct; /* size of the structure block */
} fdt_header;

typedef struct
{
	uint64_t address;
	uint64_t size;
} fdt_reserve_entry;

// struct fdt_node_header
// {
// 	uint32_t tag;
// 	char name[];
// };

typedef struct
{
	// uint32_t tag;
	uint32_t len;
	uint32_t nameoff;
	// char data[];
} fdt_property;

#define FDT_MAGIC 0xd00dfeed /* 4: version, 4: total size */
#define FDT_TAGSIZE sizeof(uint32_t)

#define FDT_BEGIN_NODE 0x00000001 /* Start node: full name */
#define FDT_END_NODE 0x00000002	  /* End node */
#define FDT_PROP 0x00000003		  /* Property: name off, size, content */
#define FDT_NOP 0x00000004		  /* nop */
#define FDT_END 0x00000009

int32_t b2l_32(int32_t input);
int64_t b2l_64(int64_t input);

// Find attribute and callback
// 1: Start address of FDT
// 2: Node name
// 3: Property name
// 4: Callback function
int fdt_find_do(void *, const char *, int (*fn)(void *, int));
int fdt_header_b2l(fdt_header *dtb_info);
// Target properties location of initrd.
// "linux,initrd-start"
// "linux,initrd-end"
#endif /* FDT_H */