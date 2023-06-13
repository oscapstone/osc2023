#include "virtual_mem.h"
#include "map_kernel.h"
#include <stdint.h>

#define null 0

struct page_table* alloc_page_table()
{
	struct page_table *pt = d_alloc(sizeof(struct page_table)) | 0xFFFF000000000000;		//allocate a virtual memory
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

		for(int level = 3;level >= 0;level--)		//search from PGD -> PTE
		{
			if(level != 0)						//not PTE
			{
				if(pt_tmp->entry[level_arr[level]] == null)		//check next_level exist
				{
					pt_tmp->entry[level_arr[level]] = ((uint64_t)alloc_page_table() & ~0xFFFF000000000000) | 0x3;	//set table_descriptor
/*
					uart_hex_64(pt_tmp->entry[level_arr[level]]);
					uart_send_string(" -> pt->entry\n");
*/
				}
				pt_tmp = pt_tmp->entry[level_arr[level]] & ~0x3 | 0xFFFF000000000000;
/*
				uart_hex_64(pt_tmp);
				uart_send_string(" -> pt\n");
*/
			}
			else	//set PTE
			{
				pt_tmp->entry[level_arr[0]] = ((pa + i * 0x1000) | PAGE_PTE_ATTR) & 0xFFFFFFFF;			//set page_descriptor
/*
				uart_hex_64(pt_tmp->entry[level_arr[0]]);
				uart_send_string(" -> PTE entry\n");
*/
			}
		}
	}
	return;
}

void copy_PGD(struct page_table *child,struct page_table *parent)
{
	struct page_table *parent_lv0 = parent;
	struct page_table *child_lv0 = child;
/*
	uart_hex_64(parent_lv0);
	uart_send_string(" -> parent_lv0\n");
	uart_hex_64(child_lv0);
	uart_send_string(" -> child_lv0\n");
*/
	for(int i=0;i<512;i++)					//PGD level
	{
		if(parent_lv0->entry[i] == null)
		{
			continue;
		}
		struct page_table *parent_lv1 = (uint64_t)parent_lv0->entry[i] & ~0x3 | 0xFFFF000000000000;
		struct page_table *child_lv1  = (uint64_t)alloc_page_table() | 0xFFFF000000000000;
		child_lv0->entry[i] = (uint64_t)child_lv1 & ~0xFFFF000000000000 | 0x3;
/*
		uart_hex_64(parent_lv1);
		uart_send_string(" -> parent_lv1\n");
		uart_hex_64(child_lv1);
		uart_send_string(" -> child_lv1\n");
*/
		for(int j=0;j<512;j++)				//PUD level
		{
			if(parent_lv1->entry[j] == null)
			{
				continue;
			}
			struct page_table *parent_lv2 = (uint64_t)parent_lv1->entry[j] & ~0x3 | 0xFFFF000000000000;
			struct page_table *child_lv2  = (uint64_t)alloc_page_table() | 0xFFFF000000000000;
			child_lv1->entry[j] = (uint64_t)child_lv2 & ~0xFFFF000000000000 | 0x3;
/*
			uart_hex_64(parent_lv2);
			uart_send_string(" -> parent_lv2\n");
			uart_hex_64(child_lv2);
			uart_send_string(" -> child_lv2\n");
*/
			for(int k=0;k<512;k++)			//PMD level
			{
				if(parent_lv2->entry[k] == null)
				{
					continue;
				}
				struct page_table *parent_lv3 = (uint64_t)parent_lv2->entry[k] & ~0x3 | 0xFFFF000000000000;
				struct page_table *child_lv3  = (uint64_t)alloc_page_table() | 0xFFFF000000000000;
				child_lv2->entry[k] = (uint64_t)child_lv3 & ~0xFFFF000000000000 | 0x3;
/*
				uart_hex_64(parent_lv3);
				uart_send_string(" -> parent_lv3\n");
				uart_hex_64(child_lv3);
				uart_send_string(" -> child_lv3\n");
*/
				for(int l=0;l<512;l++)		//PTE level
				{
					if(parent_lv3->entry[l] == null)
					{
						continue;
					}
					child_lv3->entry[l] = parent_lv3->entry[l];
/*
					uart_hex_64(child_lv3->entry[l]);
					uart_send_string(" -> entry\n");
*/
				}
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
