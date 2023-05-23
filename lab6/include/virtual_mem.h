#ifndef VIRTUAL_MEM_H
#define VIRTUAL_MEM_H
#include <stdint.h>

struct page_table
{
	uint64_t entry[512];	
};

struct page_table* alloc_page_table();
void walk(struct page_table *pt,uint64_t va,uint64_t size,uint64_t pa);
void mappage(struct page_table *pt,uint64_t va,uint64_t size,uint64_t pa);

#endif
