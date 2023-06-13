#ifndef	MM_H
#define	MM_H

#define PAGE_SHIFT	 12
#define TABLE_SHIFT 	 9
#define SECTION_SHIFT	 (PAGE_SHIFT + TABLE_SHIFT)

#define PAGE_SIZE   	 (1 << PAGE_SHIFT)	
#define SECTION_SIZE	 (1 << SECTION_SHIFT)	

/*
 * Equals to 4MB
 */
#define LOW_MEMORY       (2 * SECTION_SIZE)

/*
 * Equal to 8MB and 12MB respectively
 */
#define USER_PROGRAM_START      (char*)0x0800000
#define USER_STACK_POINTER      (char*)0x1200000

#ifndef __ASSEMBLER__

void memzero(unsigned long src, unsigned long n);

#endif /* __ASSEMBLER__ */

#endif /* MM_H */