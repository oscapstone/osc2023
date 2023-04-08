#include <stdint.h>
#ifndef MEM_H
#define MEM_H

/******************************************************************
 * Implement page frame allocator by buddy system.
 *****************************************************************/

#define MEM_MAX 7 		// 2^0 -> 2^6 * 4KB
#define MEM_SIZE 0x3c000	// 0x3c00 0000 / 4KB (0x1000)
#define FRAME_SIZE 0x1000
#define SMEM_MAX 5		// 2^0 -> 2^4 * 32 bits

typedef struct MEM_NODE{
	int index;
	struct MEM_NODE* prev;
	struct MEM_NODE* next;
}mem_node;

typedef struct SLOT{
	void* addr;
	struct SLOT* prev;
	struct SLOT* next;
}slot;

/// Page frame allocator
void *pmalloc(int);

/// Small memory allocator
void *smalloc(int);

/// Free the page
// TODO: We should free the page without the accurate memory input.
int pfree(void*);

/// Reserve the memory location.
/// start address + size
int preserve(void*, int);

/// Startup allocation.
/// This function should be called after reserve address.
int pmalloc_init(void);

#endif //MEM_H
