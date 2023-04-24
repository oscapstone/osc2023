#include "string.h"
#include "mini_uart.h"

#define null 0

extern unsigned char _end;
static char* kernel_end = (char*)&_end;

static int MAX_ORDER;

struct page
{
	int index;				//frame's index
	int val;				//page's size
	struct page* next;
	int valid;				//check for used
};

struct page** page_list;
struct page*  page_array;
static int page_total;

struct reserve_pool
{
	char* start;
	char* end;
	struct reserve_pool* next;
};

struct reserve_pool *res_pool;

struct pool_element
{
	char* start;
	int size;
	struct pool_element* next;
};

struct pool_element *pool;
struct pool_element *use_pool;

void init_buddy()
{
	page_total = (0x3C000000 - 0x0)/4096;				//245760
	char* buddy_start = memalloc(page_total * sizeof(struct page));
	MAX_ORDER = log_2(page_total);						//MAX_ORDER : 17
	page_total = pow_2(MAX_ORDER);
	page_array = buddy_start;							//page_array start from buddy_start
	page_array[0].val = log_2(page_total);				//maximum order -> can use all page_total
	page_array[0].index = 0;
	page_array[0].next = null;
	page_array[0].valid = 1;							//valid = true
	for(int i=1;i<page_total;i++)
	{
		page_array[i].val = -1;							//this page don't have value -> -1
		page_array[i].index = i;
		page_array[i].next = null;
		page_array[i].valid = 1;
	}
	page_list = memalloc((MAX_ORDER) * sizeof(struct page*));
	page_list[0] = &page_array[0];
	for(int i=1;i<MAX_ORDER;i++)
	{
		page_list[i] = null;							//unuse order_list
	}
	res_pool = null;
	pool = null;
	use_pool = null;
	int heap_end = buddy_start + page_total * sizeof(struct page) + (MAX_ORDER) * sizeof(struct page*);
	memory_reserve(kernel_end,heap_end + 0x100000);		//reserve heap size
	return;
}

char* search_best_order(int page_order)
{
	int order;
	for(order=0;order+page_order <= MAX_ORDER;order++)		//order from best_fit order to MAX_ORDER
	{
		if(page_list[MAX_ORDER-order-page_order] != null)	//this order have enough space to divide
		{
			break;
		}
	}
	struct page* start = page_list[MAX_ORDER-order-page_order];					//store best_fit page's biggest page start
	while(order != 0)
	{
		struct page* now_list = start;
		page_list[MAX_ORDER-order-page_order] = now_list->next;					//update to this order's next free page
		int size = pow_2(now_list->val - 1);									//next order's page size (div 2)
		struct page* next_page = &page_array[now_list->index + size];			//calculate next order's page start
		now_list->next = next_page;
	   	now_list->val--;														//modify top half value (div 2)
		next_page->next = null;
		next_page->val = now_list->val;											//modify bottom half value (div 2)
		order--;
		page_list[MAX_ORDER-order-page_order] = now_list;
	}
	struct page* now_list = page_list[MAX_ORDER-order-page_order];				//best_fit's list
 	page_list[MAX_ORDER-order-page_order] = now_list->next;
	for(int i=0;i<pow_2(now_list->val);i++)
	{
		(now_list + i)->valid = 0;												//update page_array's valid
	}
	return (char*)(4096 * now_list->index);										//alloc start address
}

char* page_alloc(int kb_need)
{
	kb_need = (kb_need%4 == 0) ?kb_need :kb_need+4-(kb_need%4);										//kb_need must be mul of 4
	int page_need = kb_need/4;
	page_need = (page_need == pow_2(log_2(page_need))) ?page_need :pow_2(log_2(page_need) + 1);
	int page_order = log_2(page_need);																//calculate which order_list to put
	struct reserve_pool *tmp = res_pool;
	while(1)
	{	
		char* start = search_best_order(page_order);				//ask for a page frame
		while(tmp != null)
		{
			if(start >= tmp->start && start <= tmp->end)			//check page frame is not in reserve memory
			{
				break;												//need to request another page frame
			}
			tmp = tmp->next;
		}
		if(tmp == null)												//have check all reserve memory
		{
			return start;
		}
	}
	return null;													//can't be here
}

void page_merge(int index)
{
	int order = page_array[index].val;
	int front = index - pow_2(order);
	int back  = index + pow_2(order);
	
	if(front >= 0 && !(front & pow_2(order)) && page_array[front].valid)		//can merge front block condition
	{
		page_array[index].val = -1;
		page_array[front].val += 1;
		struct page* tmp = page_list[MAX_ORDER-order];
		if(tmp == &page_array[front])											//remove page is order_list head
		{
			page_list[MAX_ORDER-order] = page_list[MAX_ORDER-order]->next;
		}
		else
		{	
			while(tmp->next != &page_array[front])								//repeat find remove page
			{
				tmp = tmp->next;
			}
			page_array[front].next = tmp->next->next;
			tmp->next = &page_array[front];
		}
		page_merge(front);														//repeat merge
	}
	else if(back < pow_2(MAX_ORDER) && (back & pow_2(order)) && page_array[back].valid)	//can merge back block condition
	{
		page_array[index].val += 1;
		page_array[back].val = -1;
		struct page* tmp = page_list[MAX_ORDER-order];
		if(tmp == &page_array[back])											//remove page is order_list head
		{
			page_list[MAX_ORDER-order] = page_list[MAX_ORDER-order]->next;
		}
		else
		{
			while(tmp->next != &page_array[back])								//repeat find remove page
			{
				tmp = tmp->next;
			}
			page_array[back].next = tmp->next->next;
			tmp->next = &page_array[back];
		}
		page_merge(index);														//repeat merge
	}
	else																		//no need to merge , reorder order_list
	{
		if(page_list[MAX_ORDER-order] == null || &page_array[index] < page_list[MAX_ORDER-order])	//insert free space in order_list head
		{
			struct page* tmp = &page_array[index];
			tmp->next = page_list[MAX_ORDER-order];
			page_list[MAX_ORDER-order] = tmp;
		}
		else																	//insert free space in order_list body
		{
			struct page* tmp = page_list[MAX_ORDER-order];
			while(tmp->next != null && tmp->next < &page_array[index])
			{
				tmp = tmp->next;
			}
			page_array[index].next = tmp->next;
			tmp->next = &page_array[index];
		}
	}
	return;
}

void page_free(char* start)
{
	struct page* tmp = start;
	int index = (int)start/4096;
	for(int i=0;i<pow_2(page_array[index].val);i++)
	{
		page_array[index + i].valid = 1;
	}
	page_merge(index);
	return;
}

void show_page()
{
	uart_send_string("************************************************************************\r\n");
	for(int i=0;i<256;i++)
	{
		uart_send_string("addr: ");
		uart_hex(4096 * page_array[i].index);
		uart_send_string("\t, index: ");
		uart_int(page_array[i].index);
		uart_send_string("\t, val: ");
		uart_int(page_array[i].val);
		uart_send_string("\t, valid: ");
		uart_int(page_array[i].valid);
		uart_send_string("\r\n");
	}
	/*log will too large
	int pos = 0;
	while(pos < page_total)
	{
		int size = pow_2(page_array[pos].val);
		for(int j=0;j<size;j++)
		{
			uart_send_string("addr: ");
			uart_hex(4096 * page_array[pos + j].index);
			uart_send_string("\t, index: ");
			uart_int(page_array[pos + j].index);
			uart_send_string("\t, val: ");
			uart_int(page_array[pos + j].val);
			uart_send_string("\t, valid: ");
			uart_int(page_array[pos + j].valid);
			uart_send_string("\r\n");
		}
		pos += size;
	}
	*/
	return;
}

void back_to_pool(struct pool_element* new_tmp)
{
	struct pool_element* tmp = pool;
	new_tmp->next = pool;
	pool = new_tmp;
	return;
}

char* d_alloc(int size)
{
	int kb_need = (size%1024 == 0) ?size/1024 :size/1024+1;			//calculate kb_need
	int page_need = kb_need/4;
	page_need = (page_need == pow_2(log_2(page_need))) ?page_need :pow_2(log_2(page_need) + 1);
	struct pool_element *tmp = pool;
	if(pool == null)												//not use yet
	{
		tmp = memalloc(sizeof(struct pool_element));				//alloc a space from heap
		tmp->start = page_alloc(kb_need);							//request a new page frame
		int index = (int)tmp->start/4096;
		tmp->size = pow_2(page_array[index].val) * 4096;
		tmp->next = null;
	}
	else if(tmp->size >= size)										//can allocate in pool's head
	{
		pool = pool->next;											//pop pool's head
		tmp->next = null;
	}
	else
	{
		int find = 0;
		while(tmp->next != null)
		{
			if(tmp->next->size >= size)								//pool have element's size >= request
			{
				find = 1;
				struct pool_element *t;
				t = tmp;
				tmp = tmp->next;									//the need pool_element
				t->next = tmp->next;								//pop pool_element
				tmp->next = null;
				break;
			}
			tmp = tmp->next;
		}
		if(find == 0)												//in pool no pool_element have request size
		{
			tmp = memalloc(sizeof(struct pool_element));			//alloc a space from heap
			tmp->start = page_alloc(kb_need);						//request a new page frame
			int index = (int)tmp->start/4096;
			tmp->size = pow_2(page_array[index].val) * 4096;
			tmp->next = null;
		}
	}
	if(tmp->size > size*2)											//will leave more than 1 page_frame
	{
		struct pool_element *new_tmp = memalloc(sizeof(struct pool_element));
		new_tmp->start = tmp->start + size;
		new_tmp->size = tmp->size - size;
		new_tmp->next = null;
		back_to_pool(new_tmp);										//give redundant block back to pool
		tmp->size = size;
	}
	tmp->next = use_pool;
	use_pool = tmp;
	return tmp->start;												//return space's start 
}

void d_free(char* start)
{
	struct pool_element *tmp = use_pool;
	if(tmp == null)
	{
		return;
	}
	else if(start == tmp->start)									//need to free use_pool head condition
	{
		use_pool = use_pool->next;
		tmp->next = null;
		back_to_pool(tmp);
		return;
	}
	else															//need to free use_pool body condition
	{
		while(tmp->next != null)
		{
			if(start == tmp->next->start)
			{
				struct pool_element *new_tmp;
				new_tmp = tmp->next;								//need to free
				tmp->next = new_tmp->next;
				new_tmp->next = null;
				back_to_pool(new_tmp);
				return;
			}
			tmp = tmp->next;
		}
	}
	return;
}

void clear_pool()
{
	while(pool != null)
	{
		page_free(pool->start);
		pool = pool->next;
	}
	while(use_pool != null)
	{
		page_free(use_pool->start);
		use_pool = use_pool->next;
	}
	return;
}

void memory_reserve(int start,int end)
{
	int start_index = start/4096;
	int end_index = end/4096;
	for(int i=start_index;i<=end_index;i++)
	{
		page_array[i].valid = 0;
	}
	struct reserve_pool *tmp = res_pool;
	
	if(tmp == null)													//no element in res_pool
	{
		struct reserve_pool *res  = memalloc(sizeof(struct reserve_pool));
		res->start = start;
		res->end = end;
		res->next = null;
		res_pool = res;
		uart_send_string("start: ");
		uart_hex(start);
		uart_send_string(" , end:");
		uart_hex(end);
		uart_send_string(" , have been reserved\r\n");
		return;
	}
	else if(end < tmp->start)										//should put in the list's head
	{
		struct reserve_pool *res = memalloc(sizeof(struct reserve_pool));
		res->start = start;
		res->end = end;
		res->next = res_pool;
		res_pool = res;
		uart_send_string("start: ");
		uart_hex(start);
		uart_send_string(" , end:");
		uart_hex(end);
		uart_send_string(" , have been reserved\r\n");
		return;
	}
	else
	{
		while(tmp != null)
		{
			if(start > tmp->end)
			{
				break;
			}
			tmp = tmp->next;
		}
	}
	struct reserve_pool *res = memalloc(sizeof(struct reserve_pool));
	res->start = start;
	res->end = end;
	res->next = tmp->next;
	tmp->next = res;
	uart_send_string("start: ");
	uart_hex(start);
	uart_send_string(" , end:");
	uart_hex(end);
	uart_send_string(" , have been reserved\r\n");
	return;
}

void show_pool_info()
{
	struct reserve_pool *tmp = res_pool;
	uart_send_string("************************************************************************\r\n");
	uart_send_string("reserve pool : \r\n");
	while(tmp != null)
	{
		uart_send_string("start: ");
		uart_hex(tmp->start);
		uart_send_string(" , end: ");
		uart_hex(tmp->end);
		uart_send_string("\r\n");
		tmp = tmp->next;
	}
	struct pool_element *use_p = use_pool;
	uart_send_string("************************************************************************\r\n");
	uart_send_string("use pool : \r\n");
	while(use_p != null)
	{
		uart_send_string("start: ");
		uart_hex(use_p->start);
		uart_send_string(" , size: ");
		uart_int(use_p->size);
		uart_send_string("\r\n");
		use_p = use_p->next;
	}
	struct pool_element *free_p = pool;
	uart_send_string("************************************************************************\r\n");
	uart_send_string("free pool : \r\n");
	while(free_p != null)
	{
		uart_send_string("start: ");
		uart_hex(free_p->start);
		uart_send_string(" , size: ");
		uart_int(free_p->size);
		uart_send_string("\r\n");
		free_p = free_p->next;
	}
	uart_send_string("************************************************************************\r\n");
	return;
}
