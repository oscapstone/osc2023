#include "mem.h"
#include "heap.h"
#include "uart.h"

#define NULL 0

/*==================================================================
 * Some private members.
 *=================================================================*/
// The memory array
// Use -1 to represent Free but belong to others body
// Use -2 to represent allocated
static char* mem_arr = 0;
static mem_node **free_mem = 0;
static slot **free_smem = 0;

static int list_add(int, int index);
static int list_delete(int, int index);
static int split_buddy(int index, int);
static int merge_buddy(int index, int v);
static int gen_slots(int);

/*=================================================================
 * Private function implementation
 *===============================================================*/
static int split_buddy(int index, int cnt){
	if(cnt >= MEM_MAX || cnt < 0)
		return 1;
	int k = 1;
	for(int i = 0; i < cnt; i ++){
		k *= 2;
	}
	cnt -= 1;
	if(index % k == 0){
		mem_arr[index + k / 2] = cnt;
		mem_arr[index] = cnt;
		for(int i = index + k / 2 + 1; i < index + k; ++i){
			mem_arr[i] = -1;
		}
		for(int i = index + 1; i < index + k / 2; ++i){
			mem_arr[i] = -1;
		}
		// Add to free list
		list_add(cnt, index);
		list_add(cnt, index + k / 2);

		// Log
		uart_puts("Frame split: ");
		uart_puth(index << 12);
		uart_puts(", size: ");
		uart_puti(k);
		uart_puts(" * 4 KB\n");
		uart_puts("New frames: ");
		uart_puth(index << 12);
		uart_puts(", ");
		uart_puth((index + k / 2) << 12);
		uart_puts("\n");

		return 0;
	}
	return 1;
}

static int merge_buddy(int index, int v){
	int t = 1;
	// The largest frame
	if(v >= MEM_MAX - 1 || v < 0)
		return 2;
	for(int i = 0; i < v; i++){
		t *= 2;
	}
	// [NF][F] -> [  LF  ]
	if(index % (t * 2) == 0 && mem_arr[index + t] == v){
		list_delete(v, index + t);
		list_delete(v, index );
		list_add(v + 1, index);
		for(int i = 1; i < t * 2; i ++){
			mem_arr[index + i] = -1;
		}
		mem_arr[index] = v + 1;
		// Log
		uart_puts("Frame merge: ");
		uart_puth(index << 12);
		uart_puts(" and ");
		uart_puth((index + t) << 12);
		uart_puts("New frame: ");
		uart_puth(index << 12);
		uart_puts(", size ");
		uart_puti(t * 2);
		uart_puts(" * 4 KB\n");
		merge_buddy(index, v + 1);
		return 0;
	}
	// [F][NF] -> [  LF  ]
	else if (index % (t * 2) == t && mem_arr[index - t] == v){
		list_delete(v, index - t);
		list_delete(v, index);
		list_add(v + 1, index - t);
		for(int i = -t + 1; i < t ; i ++){
			mem_arr[index + i] = -1;
		}
		mem_arr[index - t] = v + 1;

		// Log
		uart_puts("Frame merge: ");
		uart_puth(index << 12);
		uart_puts(" and ");
		uart_puth((index - t) << 12);
		uart_puts("New frame: ");
		uart_puth((index - t) << 12);
		uart_puts(", size ");
		uart_puti(t * 2);
		uart_puts(" * 4 KB\n");
		merge_buddy(index - t, v + 1);
		return 0;
	}
	else{
		uart_puts("Frame merge: CANNOT MERGE!\n");
	}
	return 1;
}

static int list_add(int cnt, int index){
	mem_node* cur = (mem_node*) malloc(sizeof(mem_node));
	cur->index = index;
	if(free_mem[cnt] == NULL){
		free_mem[cnt] = cur;
		cur->next = NULL;
		cur->prev = NULL;
	}
	else{
		cur->next = free_mem[cnt];
		free_mem[cnt]->prev = cur;
		free_mem[cnt] = cur;
	}
	return 0;
}

static int list_delete(int cnt, int index){
	mem_node* head = free_mem[cnt];
	mem_node* cur;

	for(cur = head; cur != NULL; cur = cur->next){
		if(index == cur->index){
			if(cur == head){
				free_mem[cnt] = cur->next;
			}
			if(cur->next != NULL){
				cur->next->prev = cur->prev;
			}
			if(cur->prev != NULL){
				cur->prev->next = cur->next;
			}
			return 0;
		}
	}
	uart_puth(index << 12);
	uart_puti(cnt);
	uart_puts("Cannot find the target node to delete\n");
	return 1;
}

static int list_pop(int cnt){
	int ret = free_mem[cnt]->index;
	list_delete(cnt, ret);
	return ret;
}

static int list_adds(int cnt, void* addr){
	slot* cur = (slot*) malloc(sizeof(slot));
	cur->addr = addr;
	if(free_smem[cnt] == NULL){
		free_smem[cnt] = cur;
		cur->next = NULL;
		cur->prev = NULL;
	}
	else{
		cur->next = free_smem[cnt];
		free_smem[cnt]->prev = cur;
		free_smem[cnt] = cur;
	}
	return 0;
}

static int list_deletes(int cnt, void* addr){
	slot* head = free_smem[cnt];
	slot* cur;

	for(cur = head; cur != NULL; cur = cur->next){
		if(addr == cur->addr){
			if(cur == head){
				free_smem[cnt] = cur->next;
			}
			if(cur->next != NULL){
				cur->next->prev = cur->prev;
			}
			if(cur->prev != NULL){
				cur->prev->next = cur->next;
			}
			return 0;
		}
	}
	uart_puth(addr );
	uart_puti(cnt);
	uart_puts("Cannot find the target node to delete\n");
	return 1;
}

static void* list_pops(int cnt){
	int ret = free_smem[cnt]->addr;
	list_deletes(cnt, ret);
	return ret;
}
/*******************************************************************
 * Generate small memorys by a 4KB frame
 * support 2^0 -> 2^4 * 32 bits
 ******************************************************************/
static int gen_slots(int size){
	int t = 1;
	for(int i = 0; i < size; ++i){
		t *= 2;
	}
	uint32_t* mem = (uint32_t*)pmalloc(0);
	for(int i = 0; i < FRAME_SIZE; i += t){
		list_adds(size, (void*)(mem + i));
	}
	uart_puts("***Slot generate size: ");
	uart_puti(t * 4);
	uart_puts(" bytes\n");
	return 0;
}

/******************************************************************
 * Page malloc init (buddy system)
 *****************************************************************/
int pmalloc_init(void){
	mem_arr = (char*)malloc(sizeof(char) * MEM_SIZE);
	free_mem = (mem_node**)malloc(sizeof(mem_node*) * MEM_MAX);
	free_smem = (slot**)malloc(sizeof(slot*) * SMEM_MAX);
	int t = 1;
	if(!mem_arr || !free_mem){
		uart_puts("Pmalloc Init error!\n");
		return 1;
	}
	memset(mem_arr, -1, MEM_SIZE);

	for(int i = 1; i < MEM_MAX; i++){
		t *= 2;
	}

	// Split the global array into max page size
	int i;
	for( i = 0; i < MEM_SIZE; i += t){
		mem_arr[i] = MEM_MAX - 1;
		list_add(MEM_MAX - 1, i);
	}
	// Generate small memory slots
	for(int i = 0; i < SMEM_MAX; i++)
		gen_slots(i);

		
	return 0;
}

/*****************************************************************
 * Page malloc
 ****************************************************************/
void* pmalloc(int cnt){
	int t = 1;
	for(int i = 0; i < cnt; i++){
		t *= 2;
	}
	if(cnt >= MEM_MAX || cnt < 0){
		uart_puts("PMALLOC invalid size!\n");
		return 0;
	}
	int ret = 0;
	if(free_mem[cnt] != NULL){
		ret = list_pop(cnt);
		for(int i = 1; i < t; i ++){
			mem_arr[ret + i] = -1;
		}
		mem_arr[ret] = -2;
		uart_puts("***PMALLOC without split!: ");
		uart_puth(ret << 12);
		uart_puts("\n");
		return (void*)(ret << 12);
	}
	// Else
	int i;
	for(i = cnt + 1; i < MEM_MAX; i ++){
		if( free_mem[i] != NULL)
			break;
	}
	for(int j = i; j > cnt; --j){
		ret = list_pop(j);
		split_buddy(ret, j);
	}
	ret = list_pop(cnt);
	for(int i = 1; i < t; i ++){
		mem_arr[ret + i] = -1;
	}
	mem_arr[ret] = -2;
	uart_puts("***PMALLOC with split!: ");
	uart_puth(ret << 12);
	uart_puts("\n");
	return (void*)(ret << 12);
}

int pfree(void* addr){
	int c = 1;
	int index = ((int) addr) >> 12;
	int t = 1;
	for(int i = 0; i < MEM_MAX; i++){
		t *= 2;
	}
	for(int i = 1; i < t; i++){
		//uart_puth((index + i) << 12);
		if(mem_arr[index + i] != (char)-1)
			break;
		c ++;
	}
	for(int i = MEM_MAX - 1; i >= 0; --i){
		if(c >> i){
			mem_arr[index] = i;
			list_add(i, index);
			uart_puts("***Free index: ");
			uart_puth(addr);
			uart_puts(", size: ");
			uart_puti(c);
			uart_puts("*4KB\n");
			merge_buddy(index, i);
			break;
		}
	}
	return 0;
}

/********************************************************************
 * page reserve
 * Each reserve is at least the largest frame size.
 ******************************************************************/
int preserve(void* addr, int size){
	int index = ((int)addr) >> 12;
	int t = 1;
	for(int i = 0; i < MEM_MAX; i++){
		t *= 2;
	}
	size = size / (t * FRAME_SIZE) + (size % t);
	size *= t;
	for(int i = 0; i < size; i ++){
		mem_arr[index + i] = -2;
	}
	uart_puts("Reserve from: ");
	uart_puth(addr);
	uart_puts(" to: ");
	uart_puth((index + size) << 12);
	uart_puts("\n");

	uart_puts("\n");
	return 0;
}

void* smalloc(int size){
	size = size / 4 + (size % 4) ? 1 : 0;	// Base 32bits
	if(size >= SMEM_MAX || size < 0)
		return 0;
	return list_pops(size);
}

/*
void* sfree(void* addr){
	return list_delete(size, addr);
}
*/
