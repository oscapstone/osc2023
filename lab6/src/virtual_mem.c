#include "virtual_mem.h"

struct page_table* PGD_alloc()
{
	struct page_table *pt = d_alloc(sizeof(struct page_table));

}

