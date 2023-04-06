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
	page_list = buddy_end + ((MAX_ORDER + 1) * sizeof(struct page*));		//put page_list behind buddy_end
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
		if(page_list[MAX_ORDER-1-order-page_order] != null)	//this list's free space , if is null -> no need to modify
		{
			break;
		}
	}
	while(order != 0)
	{
		struct page* now_list = page_list[MAX_ORDER-1-order-page_order];							//store page's start
		page_list[MAX_ORDER-1-order-page_order] = now_list->next;									//update order level's next free page
		int size = pow_2(now_list->val - 1);														//next order's page size (div 2)
		struct page* next_page = &page_array[now_list->index + size];
		now_list->next = next_page;																	//set next order's next free index
	   	now_list->val--;																			//modify top half value (div 2)
		next_page->next = null;
		next_page->val = now_list->val;																//modify bottom half value (div 2)
		order--;
		page_list[MAX_ORDER-1-order-page_order] = now_list;
	}
	struct page* now_list = page_list[MAX_ORDER-1-order-page_order];
 	page_list[MAX_ORDER-1-order-page_order] = page_list[MAX_ORDER-1-order-page_order]->next;
	now_list->valid = 0;																			//this size of page is used -> valid = false
	now_list->next = null;																			//used page's next is NULL
	return (char*)(0x10000000 + 4096 * now_list->index);											//alloc start address
}

char* page_alloc(int kb_need)
{
	kb_need = (kb_need%4 == 0) ?kb_need :kb_need+4-(kb_need%4);
	int page_need = kb_need/4;
	page_need = (page_need == pow_2(log_2(page_need))) ?page_need :pow_2(log_2(page_need) + 1);
	int page_order = log_2(page_need);					//calculate which order_list to put
	char* start = search_best_order(page_order);		//get start address of array
	return start;
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
			uart_int(page_array[pos].index + j);
			uart_send_string(" , valid: ");
			uart_int(page_array[pos].valid);
			uart_send_string("\r\n");
		}
		pos += size;
	}
	return;
}
