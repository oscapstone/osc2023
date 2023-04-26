#include "mm.h"
#include "uart.h"

page_t bookkeep[PAGE_FRAME_NUM];
free_area_t free_area[MAX_ORDER+1];

obj_allocator_t obj_alloc_pool[MAX_OBJ_ALLOCTOR_NUM];
reserved_t mem_reserved[MAX_MEM_RESERVED];

void page_init() {
	for (int i = 0; i < PAGE_FRAME_NUM; i++) {
		bookkeep[i].pfn = i;
		bookkeep[i].used = Free;
		bookkeep[i].order = -1;
		bookkeep[i].phy_addr = LOW_MEMORY + i * PAGE_SIZE;
	}
}

void free_area_init() {
	for (int i = 0; i < MAX_ORDER + 1; i++) {
		INIT_LIST_HEAD(&free_area[i].freelist);
	}

	for (int i = 0; i < PAGE_FRAME_NUM; i += MAX_ORDER_SIZE) {
		push_block_to_free_area(&bookkeep[i], &free_area[MAX_ORDER], MAX_ORDER);
	}
}

void push_block_to_free_area(page_t *pushed_block, free_area_t *fa, int order) {
	list_add_tail(&pushed_block->list, &fa->freelist);
	pushed_block->used = Free;
	pushed_block->order = order;
	fa->nr_free++;
}

void pop_block_from_free_area(page_t *pop_block, free_area_t *fa) {
	list_del(&pop_block->list);
	pop_block->used = Taken;
	fa->nr_free--;
}

struct page* buddy_block_alloc(int order) {

	uart_puts("\n[buddy_block_alloc]Before allocated memory:");
	dump_buddy();

	if ( (order<0) | (order>MAX_ORDER) ) {
	uart_puts("[buddy_block_alloc] ");
	uart_puti(order);
	uart_puts(" is invalid order!\n\r");
        return 0;
	}

	uart_puts("[buddy_block_alloc] Requested Order: ");
	uart_puti(order);
	uart_puts(", Size: ");
	uart_puti(1 << order);
	uart_puts("\n\r");

	for (int i = order; i < MAX_ORDER + 1; i++) {
		if (list_empty(&free_area[i].freelist)) continue;

		struct page *target_block = (struct page*)free_area[i].freelist.next;
		pop_block_from_free_area(target_block, &free_area[i]);
		target_block->order = order;

		for (int current_order = i; current_order > order; current_order--) {
			int downward_order = current_order - 1;

			int buddy_pfn = FIND_BUDDY_PFN(target_block->pfn, downward_order);
			struct page* bottom_half_block = &bookkeep[buddy_pfn];
			push_block_to_free_area(bottom_half_block, &free_area[downward_order], downward_order);
			
			uart_puts("Push back -> Redundant block(bottom half)  [ pfn= ");
			uart_puti(bottom_half_block->pfn);
			uart_puts(", order= ");
			uart_puti(bottom_half_block->order);
			uart_puts(" ]\n\r");

			
		}

		uart_puts("\n[buddy_block_alloc]After allocated memory:");
		dump_buddy();

		uart_puts("[buddy_block_alloc] Result - Allocated block[ pfn= ");
		uart_puti(target_block->pfn);
		uart_puts(", order= ");
		uart_puti(target_block->order);
		uart_puts(", phy_addr_16= ");
		uart_hex(target_block->phy_addr);
		uart_puts("]\n\r");
		uart_puts("[buddy_block_alloc] **done**\n\n");

		return target_block;
	}
	uart_puts("[buddy_block_alloc] No free memory space!\n\r");

	return 0;
}

void buddy_block_free(struct page* block) {
	uart_puts("\n[buddy_block_free] **Start free block[ pfn= ");
	uart_puti(block->pfn);
	uart_puts(", order= ");
	uart_puti(block->order);
	uart_puts("]**\n\r");

	uart_puts("\n[buddy_block_free]Before free memory:");
	dump_buddy();

	int buddy_pfn = FIND_BUDDY_PFN(block->pfn, block->order);
	page_t* buddy_block = &bookkeep[buddy_pfn];
	while (block->order < MAX_ORDER && block->order == buddy_block->order && buddy_block->used == Free) {
		pop_block_from_free_area(buddy_block, &free_area[buddy_block->order]);

		// Find left block as primary block
		int lbuddy_pfn = FIND_LBUDDY_PFN(block->pfn, block->order);
		block = &bookkeep[lbuddy_pfn];

		block->order++;

		buddy_pfn = FIND_BUDDY_PFN(block->pfn, block->order);
		buddy_block = &bookkeep[buddy_pfn];
	}
	push_block_to_free_area(block, &free_area[block->order], block->order);

	uart_puts("\n[buddy_block_free]After free memory:");
	dump_buddy();

	uart_puts("[buddy_block_free] **done**\n\n");
}

void dump_buddy()
{
    uart_puts("\n---------Buddy Debug---------\n");
    uart_puts("***Freelist(free_area) Debug***");
    for (int i = MAX_ORDER; i >= 0; i--) {
        uart_puts("\nOrder-");
	uart_puti(i);
	uart_puts("\n\r");

        struct list_head *pos;
        list_for_each(pos, (struct list_head *) &free_area[i].freelist) {
            uart_puts(" -> [pfn= ");
	    uart_puti(((struct page *)pos)->pfn);
	    uart_puts(", phy_addr_16= 0x");
	    uart_hex(((struct page *)pos)->phy_addr);
	    uart_puts("]");
        }
    }
    uart_puts("\n---------End Buddy Debug---------\n\n");
}

void __init_obj_alloc(obj_allocator_t *obj_allocator_p, int objsize) {
	INIT_LIST_HEAD(&obj_allocator_p->full);
	INIT_LIST_HEAD(&obj_allocator_p->partial);
	INIT_LIST_HEAD(&obj_allocator_p->empty);
	obj_allocator_p->curr_page = NULL;

	obj_allocator_p->objsize = objsize;
	obj_allocator_p->obj_per_page = PAGE_SIZE / objsize;
	obj_allocator_p->obj_used = 0;
	obj_allocator_p->page_used = 0;
}

void __init_obj_page(page_t* page_p) {
	page_p->obj_used = 0;
	page_p->free = NULL;
}

int register_obj_allocator(int objsize) {
	if (objsize < MIN_ALLOCATED_OBJ_SIZE) {
		objsize = MIN_ALLOCATED_OBJ_SIZE;
		uart_puts("[register_obj_allocator] Min object size is 8, automatically set it to 8 ");
	}

	if (objsize > MAX_ALLOCATED_OBJ_SIZE) {
		objsize = MAX_ALLOCATED_OBJ_SIZE;
		uart_puts("[register_obj_allocator] Max object size is 2048, automatically set it to 2048 ");
	}

	for (int token = 0; token < MAX_OBJ_ALLOCTOR_NUM; token++) {
		if (obj_alloc_pool[token].objsize != 0)
			continue;

		__init_obj_alloc(&obj_alloc_pool[token], objsize);

		uart_puts("[register_obj_allocator] Successfully Register object allocator! [objsize= ");
		uart_puti(objsize);
		uart_puts(", token= ");
		uart_puti(token);
		uart_puts("]\n\r");

		return token;
	}

	uart_puts("[register_obj_allocator] Allocator pool has been fully registered.");
}

void *obj_allocate(int token) {
    if (token < 0 || token >= MAX_OBJ_ALLOCTOR_NUM) {
        uart_puts("[obj allocator] Invalid token\n");
        return 0;
    }    
    
    obj_allocator_t *obj_allocator_p = &obj_alloc_pool[token];
    void *allocated_addr = NULL; // address of allocated object 

	uart_puts("[obj_allocate] Requested token: ");
	uart_puti(token);
	uart_puts(", size: ");
	uart_puti(obj_allocator_p->objsize);
	uart_puts("\n\r");

    uart_puts("[obj_allocate] Before allocation:");
    dump_obj_alloc(obj_allocator_p);
    
    if (obj_allocator_p->curr_page == NULL) {
        page_t *page_p;
        if (!list_empty(&obj_allocator_p->partial)) { 
            // Use partial allocated page
            page_p = (page_t *) obj_allocator_p->partial.next;
            list_del(&page_p->list);
        } else if (!list_empty(&obj_allocator_p->empty)) {
            // Use empty(no alloacted object) page
            page_p = (page_t *) obj_allocator_p->empty.next;
            list_del(&page_p->list);
        } else {
            // Demand a new page from buddy memory allocator
            page_p = buddy_block_alloc(0);
            __init_obj_page(page_p);
            page_p->obj_alloc = obj_allocator_p;

            obj_allocator_p->page_used += 1;
        }
        obj_allocator_p->curr_page = page_p;
    }

    /* TODO: Explan how obj_freelist work*/
    struct list_head *obj_freelist = obj_allocator_p->curr_page->free;
    if (obj_freelist != NULL) {
        // Allocate memory by free list in current page
        allocated_addr = obj_freelist;
        obj_freelist = obj_freelist->next; // Point to next address of free object;
    }
    else {
        // Allocate memory to requested object 
        allocated_addr = (void *) obj_allocator_p->curr_page->phy_addr + 
                         obj_allocator_p->curr_page->obj_used * obj_allocator_p->objsize;
    }

    obj_allocator_p->obj_used += 1;
    obj_allocator_p->curr_page->obj_used += 1;

    // Check if page full
    if (obj_allocator_p->obj_per_page == obj_allocator_p->curr_page->obj_used) {
        list_add_tail(&obj_allocator_p->curr_page->list, &obj_allocator_p->full);
        obj_allocator_p->curr_page = NULL;
    }

	uart_puts("[obj_allocate] Allocated address: [phy_addr_16= ");
	uart_hex(allocated_addr);
	uart_puts("]\n\r");


    uart_puts("[obj_allocate] After allocation:");
    dump_obj_alloc(obj_allocator_p);
    uart_puts("[obj_allocate] **Done**\n\n");
    
    return allocated_addr;
}

void obj_free(void *obj_addr) {
    // Find out corressponding page frame number and object allocator it belongs to.
    int obj_pfn = PHY_ADDR_TO_PFN(obj_addr);
    page_t *page_p = &bookkeep[obj_pfn];
    obj_allocator_t *obj_allocator_p = page_p->obj_alloc;
    
    uart_puts("\n[obj_free] Free object procedure!\n");
	uart_puts("[obj_free] Page info: 0x");
	uart_hex(obj_addr);
	uart_puts(" [pfn= ");
	uart_puti(obj_pfn);
	uart_puts(", obj_used= ");
	uart_puti(page_p->obj_used);
	uart_puts("]\n\r");

	uart_puts("[obj_free] object free list point to [0x");
	uart_hex(page_p->free);
	uart_puts("]\n\r");
    
    uart_puts("[obj_free] Before free:");
    dump_obj_alloc(obj_allocator_p);
    

    // Make page's object freelist point to address of new first free object.
    // And the contect of released object should record the orginal address 
    // of first free object that object freelist point to.
    // As the result, if we want to access second free object, just 
    // using free->next(first 8 bytes) to get expected address. 
    // So we can link and access all free object by this strategy without extra
    // moemory space.
    struct list_head *temp = page_p->free;
    page_p->free = (struct list_head *) obj_addr;
    page_p->free->next = temp;

    obj_allocator_p->obj_used -= 1;
    page_p->obj_used -= 1;

    // From full to partial 
    if (obj_allocator_p->obj_per_page-1 == page_p->obj_used) {
        list_del(&page_p->list); // pop out from full list
        list_add_tail(&page_p->list, &obj_allocator_p->partial); // add to partial list 
    }

    // From partial to empty
    // and make sure this page not currently used by object allocator
    if (page_p->obj_used == 0 && obj_allocator_p->curr_page != page_p) {
        list_del(&page_p->list); // pop out from partial list
        list_add_tail(&page_p->list, &obj_allocator_p->empty);
    }

    
    uart_puts("[obj_free] After free:");
    dump_obj_alloc(obj_allocator_p);
    uart_puts("[obj_free] **Done**\n\n");
}

void dump_obj_alloc(obj_allocator_t *obj_allocator_p)
{
    uart_puts("\n---------Object Allocator Debug---------\n");
	uart_puts("objsize = ");
	uart_puti(obj_allocator_p->objsize);
	uart_puts("\n\r");
	uart_puts("obj_per_page = ");
	uart_puti(obj_allocator_p->obj_per_page);
	uart_puts("\n\r");
	uart_puts("obj_used = ");
	uart_puti(obj_allocator_p->obj_used);
	uart_puts("\n\r");
	uart_puts("page_used = ");
	uart_puti(obj_allocator_p->page_used);
	uart_puts("\n\r");
    
    uart_puts("\nobject_allocator->curr_page current page info:\n");
    if (obj_allocator_p->curr_page != NULL) {
		uart_puts("obj_allocator_p->curr_page = [0x");
		uart_hex(obj_allocator_p->curr_page->phy_addr);
		uart_puts("]\n\r");
		uart_puts("obj free list point to = [0x");
		uart_hex(obj_allocator_p->curr_page->free);
		uart_puts("]\n\r");
		uart_puts("obj_used = ");
		uart_puti(obj_allocator_p->curr_page->obj_used);
		uart_puts("\n\r");
		uart_puts("pfn = ");
		uart_puti(obj_allocator_p->curr_page->pfn);
		uart_puts("\n\r");
    }
    else {
        uart_puts("object_allocator->curr_page is NULL currently\n");
    }
    uart_puts("\n\r");

    struct list_head *pos;
    uart_puts("object_allocator->full list:\n");
    list_for_each(pos, (struct list_head *) &obj_allocator_p->full) {
		uart_puts("--> [pfn= ");
		uart_puti(((struct page*) pos)->pfn);       
		uart_puts("]");
    }
    uart_puts("\n");

    uart_puts("object_allocator->partial list:\n");
    list_for_each(pos, (struct list_head *) &obj_allocator_p->partial) {
		uart_puts("--> [pfn= ");
		uart_puti(((struct page*) pos)->pfn);
		uart_puts("]");
    }
    uart_puts("\n");

    uart_puts("object_allocator->empty list:\n");
    list_for_each(pos, (struct list_head *) &obj_allocator_p->empty) {
        uart_puts("--> [pfn= ");
		uart_puti(((struct page*) pos)->pfn);
		uart_puts("]");
    }

    uart_puts("\n---------End Object Allocator Debug---------\n\n");
}

void __init_kmalloc()
{
    for (int i = MIN_KMALLOC_ORDER;i <= MAX_KMALLOC_ODER;i++) {
        register_obj_allocator(1 << i);
    }
}

void *kmalloc(int size)
{
	uart_puts("[kmalloc] Requested Size: ");
	uart_puti(size);
	uart_puts("\n\r");

    void *allocated_addr;

    // Object allocator
    for (int i = MIN_KMALLOC_ORDER;i <= MAX_KMALLOC_ODER;i++) {
        if (size <= (1<<i)) {
            allocated_addr = obj_allocate(i - MIN_KMALLOC_ORDER);

            uart_puts("[kmlloc] Allocated address: 0x");
			uart_hex(allocated_addr);
			uart_puts("\n\r");
            uart_puts("[kmlloc] **Done**\n\n");

            return allocated_addr;
        }
    }
    // Buddy Memory allocator
    for (int i = 0;i <= MAX_ORDER;i++) {
        if (size <= 1<<(i + PAGE_SHIFT)) {
            allocated_addr = (void *) buddy_block_alloc(i)->phy_addr;

            uart_puts("[kmlloc] Allocated address: 0x");
			uart_hex(allocated_addr);
			uart_puts("\n\r");
            uart_puts("[kmlloc] **Done**\n\n");

            return allocated_addr;
        }
    }
    
	uart_puts("[kmalloc] ");
	uart_puti(size);
	uart_puts(" Bytes too large!\n\r");

    return NULL;
}

void kfree(void *addr) 
{
	uart_puts("[kfree] Free Memory Address: 0x");
	uart_hex(addr);
	uart_puts("\n\r");

    int pfn = PHY_ADDR_TO_PFN(addr);
    page_t *page_p = &bookkeep[pfn];

    if (page_p->obj_alloc != NULL) {
        // Belongs to Object Allocator
        obj_free(addr);
    } else {
        // Belongs to Buddy Memory Allocator
        buddy_block_free(page_p);
    }

    uart_puts("[kfree] **Done**\n\n");
}

void mem_reserved_init(){
	for(int i = 0; i < MAX_MEM_RESERVED; i++){
		mem_reserved[i].is_reserved = 0;
		mem_reserved[i].start = 0x0;
		mem_reserved[i].offset = 0x0;
	}
}

void put_memory_reserve(unsigned start, unsigned end){
	int i = 0;

	for(i; i < MAX_MEM_RESERVED; i++){
		if(mem_reserved[i].is_reserved == 0) break;
	}

	mem_reserved[i].is_reserved = 1;
	mem_reserved[i].start = start - (start % 0x200000);
	mem_reserved[i].offset = (end - start);
}

struct page * reserve_memory_block(reserved_t * reserved){
	int order = round_up_to_order((reserved->offset) >> PAGE_SHIFT);
	uart_puts("A reserve a page with order ");
	uart_puti(order);
	struct list_head * curr = free_area[MAX_ORDER].freelist.next;
	while (((page_t *)curr)->phy_addr != reserved->start) {
		curr = curr->next;
	}
	uart_puts(" at start point: ");
	uart_hex(((page_t *)curr)->phy_addr);
	uart_puts("\n");

	struct page * target = (page_t *)curr;

	for(int j = MAX_ORDER; j > order; j--){
		int downward_buddy_pfn = FIND_BUDDY_PFN(target->pfn, j-1);
		uart_puts("\nDownward buddy pfn: ");
		uart_puti(downward_buddy_pfn);
		struct page * downward_buddy = &bookkeep[downward_buddy_pfn];
		push_block_to_free_area(downward_buddy, &free_area[j-1], j-1);
		uart_puts("\nSplitted into 2 chunks in order: ");
		uart_puti(j-1);
	}

	uart_puts("\nFinally puts chunk in memory:");
	uart_hex(target->phy_addr);
	dump_buddy();
	return target;
}

void apply_memory_reserve(){
	for(int i = 0; i < MAX_MEM_RESERVED; i++){
		if(mem_reserved[i].is_reserved == 0) break;
		else reserve_memory_block(&mem_reserved[i]);	
	}
}

int round_up_to_order(int page_num){
	int ret = 0;
	page_num = page_num >> 1; 
	while(1){
		if(page_num==0) break;
		else { 
			page_num = page_num >> 1;
			ret += 1;
		}
	}
	return ret;
}

void dump_mem_reserved(){
	for(int i = 0; i < MAX_MEM_RESERVED; i++){
		uart_puts("Index: ");
		uart_puti(i);
		uart_puts("\tis_reserved: ");
		uart_puti(mem_reserved[i].is_reserved);
		uart_puts("\tStart: ");
		uart_hex(mem_reserved[i].start);
		uart_puts("\tOffset: ");
		uart_hex(mem_reserved[i].offset);
		uart_puts("\n");
	}
}

void mm_init()
{
    page_init();
    free_area_init();
    /**
     *  Test Buddy memory Allocator
     */
    int allocate_test1[] = {5, 0, 6, 3, 0};
    int test1_size = sizeof(allocate_test1) / sizeof(int);
    page_t *(one_pages[test1_size]);
    for (int i = 0;i < test1_size;i++) {
        page_t *one_page = buddy_block_alloc(allocate_test1[i]); // Allocate one page frame
        one_pages[i] = one_page;
    }
    buddy_block_free(one_pages[2]);
    buddy_block_free(one_pages[1]);
    buddy_block_free(one_pages[4]);
    buddy_block_free(one_pages[3]);
    buddy_block_free(one_pages[0]);
}

void dyn_init() {
    page_init();
    free_area_init();
    
    __init_kmalloc();
	void *address_1 = kmalloc(16);
    void *address_2 = kmalloc(64);
    kfree(address_1);
    void *address_3 = kmalloc(1024);
    kfree(address_2);
    kfree(address_3);
    void *address_4 = kmalloc(16);
    void *address_9 = kmalloc(16384);
    void *address_10 = kmalloc(16384);
    kfree(address_4);
    void *address_5 = kmalloc(32);
    void *address_6 = kmalloc(32);
    kfree(address_5);
    kfree(address_6);
    void *address_7 = kmalloc(512);
    void *address_8 = kmalloc(512);
    kfree(address_8);
    kfree(address_7);
    kfree(address_9);
    kfree(address_10);
    void *address_11 = kmalloc(8192);
    void *address_12 = kmalloc(65536);
    void *address_13 = kmalloc(128);
    kfree(address_11);
    void *address_14 = kmalloc(65536);
    kfree(address_13);
    kfree(address_12);
    kfree(address_14);
    void *address_15 = kmalloc(256);
    kfree(address_15);
}
