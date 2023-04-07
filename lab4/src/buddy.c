#include "string.h"
#include "mini_uart.h"

#define MAX_ORDER	8		//(4KB * 2^0 ~ 4KB * 2^7) -> (4KB ~ 512KB)
#define null 0

struct page
{
	int index;				//frame's index
	int val;				//page's size
	struct page* next;
	int valid;				//check for used
};

struct page** page_list;
struct page*  page_array;
static int page_total = 512/4;	//page_size = 4KB

void init_buddy()
{
	char* buddy_start = (char*)0x10000000;				//can use 0x10000000 ~ 0x20000000
	char* buddy_end = buddy_start + (page_total * sizeof(struct page));
	page_array = buddy_start;							//page_array start from buddy_start
	page_array[0].val = log_2(page_total);				//maximum value -> can use all page_total
	page_array[0].index = 0;
	page_array[0].next = null;							//init next is NULL
	page_array[0].valid = 1;							//valid = true
	for(int i=1;i<page_total;i++)
	{
		page_array[i].val = -1;							//this page don't have value -> -1
		page_array[i].index = i;
		page_array[i].next = null;						//init next is NULL
		page_array[i].valid = 1;
	}
	page_list = buddy_end + ((MAX_ORDER) * sizeof(struct page*));		//put page_list behind buddy_end
	page_list[0] = &page_array[0];											//for tmp , no use
	for(int i=1;i<MAX_ORDER;i++)
	{
		page_list[i] = null;							//init list -> list have unused space
	}
	return;
}

char* search_best_order(int page_order)
{
	int order;
	for(order=0;order+page_order < MAX_ORDER;order++)		//order from best_fit order to MAX_ORDER
	{
		if(page_list[MAX_ORDER-1-order-page_order] != null)	//this order have enough space to divide
		{
			break;
		}
	}
	struct page* start = page_list[MAX_ORDER-1-order-page_order];									//store best_fit page's biggest page start
	while(order != 0)
	{
		struct page* now_list = start;
		page_list[MAX_ORDER-1-order-page_order] = now_list->next;									//update to this order's next free page
		int size = pow_2(now_list->val - 1);														//next order's page size (div 2)
		struct page* next_page = &page_array[now_list->index + size];								//calculate next order's page start
		now_list->next = next_page;
	   	now_list->val--;																			//modify top half value (div 2)
		next_page->next = null;
		next_page->val = now_list->val;																//modify bottom half value (div 2)
		order--;
		page_list[MAX_ORDER-1-order-page_order] = now_list;
	}
	struct page* now_list = page_list[MAX_ORDER-1-order-page_order];								//best_fit's list
 	page_list[MAX_ORDER-1-order-page_order] = now_list->next;
	for(int i=0;i<pow_2(now_list->val);i++)
	{
		(now_list + i)->valid = 0;																	//update page_array's valid
	}
	now_list->next = null;																			//clear occupy page's next
	return (char*)(0x10000000 + 4096 * now_list->index);											//alloc start address
}

char* page_alloc(int kb_need)
{
	kb_need = (kb_need%4 == 0) ?kb_need :kb_need+4-(kb_need%4);										//kb_need must be mul of 4
	int page_need = kb_need/4;
	page_need = (page_need == pow_2(log_2(page_need))) ?page_need :pow_2(log_2(page_need) + 1);
	int page_order = log_2(page_need);																//calculate which order_list to put
	char* start = search_best_order(page_order);													//get start address of array
	return start;
}

void page_merge(int index)
{
	int order = page_array[index].val;
	int front = index - pow_2(order);
	int back = index + pow_2(order);
	
	if(front >= 0 && !(front & pow_2(order)) && page_array[front].valid)		//can merge front block condition
	{
		page_array[index].val = -1;
		page_array[front].val += 1;
		struct page* tmp = page_list[MAX_ORDER-1-order];
		if(tmp == &page_array[front])											//remove page is order_list head
		{
			page_list[MAX_ORDER-1-order] = page_list[MAX_ORDER-1-order]->next;
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
		page_merge(index);														//repeat merge
	}
	else if(back < 128 && (back & pow_2(order)) && page_array[back].valid)		//can merge back block condition
	{
		page_array[index].val += 1;
		page_array[back].val = -1;
		struct page* tmp = page_list[MAX_ORDER-1-order];
		if(tmp == &page_array[back])											//remove page is order_list head
		{
			page_list[MAX_ORDER-1-order] = page_list[MAX_ORDER-1-order]->next;
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
		if(page_list[MAX_ORDER-1-order] == null || &page_array[index] < page_list[MAX_ORDER-1-order])	//insert free space in order_list head
		{
			struct page* tmp = &page_array[index];
			tmp->next = page_list[MAX_ORDER-1-order];
			page_list[MAX_ORDER-1-order] = tmp;
		}
		else																	//insert free space in order_list body
		{
			struct page* tmp = page_list[MAX_ORDER-1-order];
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

void page_free(int index)
{
	for(int i=0;i<pow_2(page_array[index].val);i++)
	{
		page_array[index + i].valid = 1;
	}
	page_merge(index);
	return;
}

void show_page()
{
	uart_send_string("buddy address start from : 0x10000000\r\n");
	int pos = 0;
	while(pos < page_total)
	{
		int size = pow_2(page_array[pos].val);
		for(int j=0;j<size;j++)
		{
			uart_send_string("index: ");
			uart_int(page_array[pos + j].index);
			uart_send_string("\t, val: ");
			uart_int(page_array[pos + j].val);
			uart_send_string("\t, valid: ");
			uart_int(page_array[pos + j].valid);
			uart_send_string("\r\n");
		}
		pos += size;
	}
	return;
}
