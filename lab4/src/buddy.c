

#define MAX_ORDER	8		//(4KB * 2^0 ~ 4KB * 2^7) -> (4KB ~ 512KB)

//each page = 4KB
struct page
{
	int order;
	int index;				//frame's index
	int val;				//page's size
	struct page* next;
}

struct buddy_order
{
	struct buddy_order* next;
}

struct buddy_order free_block[MAX_ORDER];		//create one linked-list for each size

void init_buddy()
{
	for(int i=0;i<MAX_ORDER;i++)
	{
		free_block[i].next = &free_block[i];
	}
	return;
}


