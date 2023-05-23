#include "virtual_mem.h"
#include "map_kernel.h"
#include <stdint.h>

#define null 0

struct page_table* alloc_page_table()
{
	struct page_table *pt = d_alloc(sizeof(struct page_table));		//allocate a virtual memory
	for(int i=0;i<512;i++)
	{
		pt->entry[i] = null;
	}
	return pt;
}

void walk(struct page_table *pt,uint64_t va,uint64_t size,uint64_t pa)
{
	int page_need = (size % 0x1000 != 0) ?(size / 0x1000) + 1 :(size / 0x1000);
	for(int i=0;i<page_need;i++)
	{
		if(i != 0)
		{
			va += 0x1000;						//next page
		}
		int PGD_offset = (va >> 39) & 0x1FF;	//9 bits
		int PUD_offset = (va >> 30) & 0x1FF;
		int PMD_offset = (va >> 21) & 0x1FF;
		int PTE_offset = (va >> 12) & 0x1FF;
		int level_arr[4] = {PTE_offset,PMD_offset,PUD_offset,PGD_offset};
		struct page_table *pt_tmp = pt;

		for(int level=3;level >= 0;level--)		//search from PGD -> PTE
		{
			if(level != 0)						//not PTE
			{
				if(pt_tmp->entry[level_arr[level]] == null)		//check next_level exist
				{
					pt_tmp->entry[level_arr[level]] = ((uint64_t)alloc_page_table() & ~0xFFFF000000000000) | 0x3;	//set table_descript_tmpor
				}
				pt_tmp = pt_tmp->entry[level_arr[level]] & ~0x3 | 0xFFFF000000000000;
			}
			else	//set PTE
			{
				pt_tmp->entry[level_arr[0]] = ((pa + i * 0x1000) | PAGE_PTE_ATTR) & 0xFFFFFFFF;			//set page_descript_tmpor
			}
		}
	}
	return;
}

void mappage(struct page_table *pt,uint64_t va,uint64_t size,uint64_t pa)
{
	walk(pt,va,size,pa);
	return;
}
