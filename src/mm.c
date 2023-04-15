#include "mm.h"
#include "uart.h"
#include "utils.h"
#include "stdint.h"
#include "initramfs.h"
#include "dtb.h"
//https://sourceware.org/binutils/docs/ld/Source-Code-Reference.html
//defined in linker script
extern char __heap_start, __heap_end;
buddy_t _buddy;
mem_pool_t _mem_pool;

void *simple_malloc(unsigned long long size) {
    static unsigned long long allocated = 0;
    if (allocated + size > (& __heap_end - & __heap_start)) {
        uart_write_string("heap is full!\n");
        return NULL;
    }
    char *ret = & __heap_start + allocated;
    disable_local_all_interrupt();
    allocated += size;
    enable_local_all_interrupt();
    return ret;
}

/////////////////////// buddy ////////////////////////
static void _buddy_blk_show(struct buddy *self, bd_blk_t *blk)
{
    int index = blk - self->blk_meta;
    uart_write_string("block index: ");
    uart_write_no(index);
    uart_write_string("\n");

    int order = BLK_ORDER(*blk);
    uart_write_string("order: ");
    uart_write_no(order);

    uart_write_string(", in_used: ");
    uart_write_string(BLK_INUSE(*blk) ? "True\n" : "False\n");

    uart_write_string("block start address: 0x");
    uart_write_no_hex(index2addr(self, index));
    uart_write_string("\n");

    uart_write_string("block end address: 0x");
    uart_write_no_hex( index2addr(self, (index + (1 << order))) );
    uart_write_string("\n");
}

static void _buddy_push_block2list(struct buddy *self, bd_blk_t *blk, int order)
{
#ifdef PRINT_LOG
    // if (BLK_INUSE(*blk)) {
        // uart_write_string("push:");
        // _buddy_blk_show(self, blk);
    // }
#endif
    SET_FREE(*blk);
    SET_ORDER(*blk, order);
    int index = blk - self->blk_meta;
    if (self->link_size[order] == 0) {
        blk->next_free_index = index;
        blk->prev_free_index = index;
    } else {
        blk->next_free_index = (self->blk_meta[self->free_head[order]]).next_free_index;
        blk->prev_free_index = (self->free_head[order]);
        (self->blk_meta[(self->blk_meta[self->free_head[order]]).next_free_index]).prev_free_index = index;
        (self->blk_meta[self->free_head[order]]).next_free_index = index;
    }
    self->free_head[order] = index;
    self->link_size[order]++;
}

static void _buddy_unlink_blk(struct buddy *self, bd_blk_t *blk, int order)
{
    //assert self->free_head[order] != NULL

    int index = blk - self->blk_meta;
#ifdef PRINT_LOG
    if (self->link_size[order] == 0) {
        uart_write_string("trying to unlink an empty queue!!!!!!!!!!!!!!!!\n");
        return;
    }
    // uart_write_string("unlink:");
    // _buddy_blk_show(self, blk);
#endif
    //blk is the first block in free_head[order]
    if (self->free_head[order] == index) {
        self->free_head[order] = (self->blk_meta[self->free_head[order]]).next_free_index;
    }
    //unlink blk
    (self->blk_meta[blk->prev_free_index]).next_free_index = blk->next_free_index;
    (self->blk_meta[blk->next_free_index]).prev_free_index = blk->prev_free_index;
    self->link_size[order]--;
    SET_INUSE(*blk);
}

void *_buddy_alloc_pages(struct buddy *self, size_t page_no)
{
    disable_local_all_interrupt();
#ifdef PRINT_LOG
    uart_write_string("---------------------------------------------\nCall Buddy Allocate pages:");
    uart_write_no(page_no);
    uart_write_string("\n");
#endif
    if (page_no == 0) {
        enable_local_all_interrupt();
        return NULL;
    }
    //align to closet power of 2
    unsigned page_cnt = alignToNextPowerOf2(page_no);
    //page_cnt is in power of 2
    unsigned request_order = ul_log2(page_cnt);
    if (request_order >= BUDDY_ORDERS) {
        enable_local_all_interrupt();
        return NULL;
    }
#ifdef PRINT_LOG
    uart_write_string("Request order: ");
    uart_write_no(request_order);
    uart_write_string("\n");
#endif
    //find a free block
    bd_blk_t *blk = NULL;
    unsigned order = request_order;
    for (; order < BUDDY_ORDERS; order++) {
        if (self->link_size[order]) {
            blk = &(self->blk_meta[self->free_head[order]]);
            _buddy_unlink_blk(self, blk, order);
            break;
        }
    }
    if (blk == NULL) {
        //cannot find a large enough free block
        enable_local_all_interrupt();
        return NULL;
    }
    int index = (blk - self->blk_meta);

    ///////////////////// split block and allocate //////////////////
    //split if necessary
    while (order > request_order) {
        order--;
        int buddy_index = (index ^ (1 << order));
        //push the buddy to free list
        _buddy_push_block2list(self, &(self->blk_meta[buddy_index]), order);
#ifdef PRINT_LOG
        uart_write_string("splited free buddy block order: \n");
        _buddy_blk_show(self, &(self->blk_meta[buddy_index]));
#endif
    }
    //resize blk itself
    SET_ORDER(*blk, order);
    // SET_INUSE(*blk);
#ifdef PRINT_LOG
    uart_write_string("Allocate block: \n");
    _buddy_blk_show(self, blk);
#endif
    ///////////////////////////////////////////////////////////////
    enable_local_all_interrupt();
    return index2addr(self, index);
}



void _buddy_free(struct buddy *self, void *addr)
{
    disable_local_all_interrupt();
    int index = addr2index(self, addr);
    //no double free
    bd_blk_t *blk = &(self->blk_meta[index]);
#ifdef PRINT_LOG
    uart_write_string("------------------------------------\nCall Buddy Free: 0x");
    uart_write_no_hex((uint64_t)addr);
    uart_write_string("\nCorrespondent Block:\n");
    _buddy_blk_show(self, blk);
#endif
    if (BLK_ISFREE(*blk)) {
        enable_local_all_interrupt();
        return;
    }
    //set blk as a free block
    int order = BLK_ORDER(*blk);
    _buddy_push_block2list(self, blk, order);
    //try to consolidate
    while (order < BUDDY_ORDERS) {
        int buddy_index = ((index ^ (1 << order)));
        bd_blk_t *buddy_blk = &(self->blk_meta[buddy_index]);
#ifdef PRINT_LOG
        uart_write_string("index: ");
        uart_write_no(index);
        uart_write_string(", buddy index :");
        uart_write_no(buddy_index);
        uart_write_string("\n");
#endif
        if (BLK_ISFREE(*buddy_blk) && BLK_ORDER(*buddy_blk) == order) {
#ifdef PRINT_LOG
            uart_write_string("buddy index is free, merge.............................\n");
#endif
            _buddy_unlink_blk(self, buddy_blk, order);
            _buddy_unlink_blk(self, blk, order);
            //Merge the two blocks
            if (buddy_index < index) {
                //buddy is the former
                // index = buddy_index;
                SWAP(index, buddy_index);
                // blk = buddy_blk;
                SWAP(blk, buddy_blk);
            }
            order++;
            _buddy_push_block2list(self, blk, order);
#ifdef PRINT_LOG
            uart_write_string("\nnew block order: ");
            uart_write_no(index);
            uart_write_string("\n");
#endif
        } else {
            //Cannot merge: not of the same size or buddy is not free
            break;
        }
    }
    // SET_ORDER(*blk, order);
    // _buddy_push_block2list(self, blk, order);
#ifdef PRINT_LOG
    uart_write_string("Push Merged Free block: \n");
    _buddy_blk_show(self, blk);
#endif
    enable_local_all_interrupt();
}

//top-down allocate block
static void _buddy_internal_mem_reserve(struct buddy *self, int start, int end, int l, int order)
{
    int r = l + (1 << order);
#ifdef PRINT_LOG
    uart_write_string("Call internal memory reserve: ");
    uart_write_string("start: ");
    uart_write_no(start);
    uart_write_string(", end: ");
    uart_write_no(end);
    uart_write_string(", l: ");
    uart_write_no(l);
    uart_write_string(", r: ");
    uart_write_no(r);
    uart_write_string("\n");
#endif
    if (start < l || end > r || start >= end) {
        //not in range
        return;
    }
    if (start == l && end == r) {
#ifdef PRINT_LOG
        if (BLK_INUSE(self->blk_meta[start])) {
            uart_write_string("Found Reserve block BUT INUSE!!!: \n");
            _buddy_blk_show(self, &(self->blk_meta[start]));
            return;
        }
#endif
        //assert(the block is free and order is correct)
        //mark this block as in use
        _buddy_unlink_blk(self, &(self->blk_meta[start]), order);
#ifdef PRINT_LOG
        uart_write_string("Reserve block: \n");
        _buddy_blk_show(self, &(self->blk_meta[start]));
#endif
        return;
    }
    bd_blk_t *front = &(self->blk_meta[l]);
    int mid = l + (1 << (order-1));
    bd_blk_t *rear = &(self->blk_meta[mid]);
    //if memory has not been split and is free
    if (BLK_ISFREE(*front) && BLK_ORDER(*front) == order) {
        //block is free
        //split current block [l, r) into [l, mid), [mid, r)
        //unlink [l, r) from free list
        _buddy_unlink_blk(self, front, order);
        //[l, mid)
        _buddy_push_block2list(self, front, order-1);
        //[mid, r)
        _buddy_push_block2list(self, rear, order-1);
    }

    _buddy_internal_mem_reserve(self, start, min(mid, end), l, order-1);
    _buddy_internal_mem_reserve(self, min(mid, end), end, mid, order-1);
}

void _buddy_mem_reserve(struct buddy *self, char *start, char *end)
{
    disable_local_all_interrupt();
    if (start < self->pool || end > alignToNextPowerOf2(self->pool_end))
        return;
    //find start index
    int start_index = (start - self->pool) / PAGE_SIZE;
    //find end index
    // int end_index = (end - self->pool) / PAGE_SIZE + ((end - self->pool) % PAGE_SIZE > 0);
    int end_index = ceil((end - self->pool), PAGE_SIZE);
    end_index = min(end_index, (1 << (BUDDY_ORDERS)));
    _buddy_internal_mem_reserve(self, start_index, end_index, 0, BUDDY_ORDERS-1);
    enable_local_all_interrupt();
}
extern char _proc_start, _proc_end;

void init_buddy(buddy_t *self)
{
    self->blk_meta = (bd_blk_t *)simple_malloc((1 << BUDDY_ORDERS) * sizeof(bd_blk_t));
    my_bzero(self->blk_meta, sizeof(self->blk_meta));
    //not in use and order is 14
    self->blk_meta[0].flags = BUDDY_ORDERS-1;
    self->blk_meta[0].next_free_index = 0;
    self->blk_meta[0].prev_free_index = 0;
    for (int i = 0; i < BUDDY_ORDERS; i++) {
        self->link_size[i] = 0;
    }
    self->link_size[BUDDY_ORDERS-1] = 1;
    self->free_head[BUDDY_ORDERS-1] = 0;

    self->pool = (char *)MEM_START;
    self->pool_end = (char *)MEM_END;
    self->alloc_pages = _buddy_alloc_pages;
    self->free = _buddy_free;
    self->mem_reserve = _buddy_mem_reserve;
    //all reserve areas
    self->mem_reserve(self, self->pool_end, alignToNextPowerOf2(self->pool_end));
    //Spin tables for multicore boot (0x0000 - 0x1000)
    self->mem_reserve(self, 0x0000, 0x1000);
    //Kernel image in the physical memory
    self->mem_reserve(self, &_proc_start, &_proc_end);
    //Initramfs
    self->mem_reserve(self, cpio_addr, cpio_end);
    //Devicetree (Optional, if you have implement it)
    //make sure this function is called after _fdt.fdt_traverse(&_fdt, initramfs_fdt_cb, NULL).
    self->mem_reserve(self, _fdt.head_addr, _fdt.end_addr);
    //Your simple allocator (startup allocator)
    //No. It's within kernel
}

void *alloc_pages(size_t no_pages)
{
    return _buddy.alloc_pages(&_buddy, no_pages);
}

void free_page(void *addr)
{
    _buddy.free(&_buddy, addr);
}
////////////////////// dynamic memory allocator ////////////////////
static void _mem_chunk_push(mem_chunk_t **head, mem_chunk_t *chunk, int order)
{
    chunk->next = *head;
    SET_CHUNK_ORDER(chunk, order);
    *head = chunk;
}

static void *_mem_chunk_pop(mem_chunk_t **head)
{
    if (*head == NULL) return NULL;
    mem_chunk_t *ret = *head;
    *head = GET_CHUNK_NEXT(ret);
    CLR_CHUNK_NEXT(ret);
    return (void *)ret->data;
}

static void _mem_chunk_pool_append(struct mem_pool *self, int order)
{
    //get a new page
    void *new_page = alloc_pages(1);
    //split it into chunks and push into free list list
    char *end = (char *)new_page + PAGE_SIZE;
    char *start = (char *)new_page;
    const size_t unit = (0x10 * order + sizeof(mem_chunk_t));
    for (; start < end; start += unit) {
        _mem_chunk_push(&(self->free_list[order]), (mem_chunk_t *)start, order);
    }
}
void *_mem_pool_malloc(struct mem_pool *self, size_t size)
{
    disable_local_all_interrupt();
    int order = ceil(size, 0x10);
#ifdef PRINT_LOG
    uart_write_string("Call mem pool kmalloc: ");
    uart_write_no(size);
    uart_write_string("\n");
#endif
    if (order >= self->chunk_orders) {
        //request size larger than max chunk size
        enable_local_all_interrupt();
        return NULL;
    }
    if (self->free_list[order] == NULL)
        _mem_chunk_pool_append(self, order);
    void *ret = _mem_chunk_pop(&(self->free_list[order]));
#ifdef PRINT_LOG
    uart_write_string("Return address: 0x");
    uart_write_no_hex((unsigned long long)ret);
    uart_write_string("\n");
#endif
    enable_local_all_interrupt();
    return ret;
}

void _mem_pool_free(struct mem_pool *self, void *addr)
{
    disable_local_all_interrupt();
    // mem_chunk_t *header = (mem_chunk_t *)container_of(addr, mem_chunk_t, data);
    mem_chunk_t *header = (mem_chunk_t *)((char *)addr - sizeof(mem_chunk_t));
#ifdef PRINT_LOG
    uart_write_string("Call mem pool free: ");
    uart_write_no_hex((unsigned long long)addr);
    uart_write_string("\n");
#endif
    int order = GET_CHUNK_ORDER(header);
    _mem_chunk_push(&(self->free_list[order]), header, order);
    enable_local_all_interrupt();
}

void init_mem_pool(mem_pool_t *self)
{
    for (int i = 0; i < CHUNK_ORDERS; i++) {
        //allocate on demand
        self->free_list[i] = NULL;
    }
    self->chunk_orders = CHUNK_ORDERS;
    self->malloc = _mem_pool_malloc;
    self->free = _mem_pool_free;
}

void *kmalloc(size_t size)
{
    return _mem_pool.malloc(&_mem_pool, size);
}

void kfree(void *addr)
{
    _mem_pool.free(&_mem_pool, addr);
}
///////////////// test function /////////////////
void test_buddy()
{
    void *p1 = alloc_pages(1);
    void *p2 = alloc_pages(1);
    void *p3 = alloc_pages(1);
    void *p4 = alloc_pages(1);
    void *p5 = alloc_pages(1);
    void *p6 = alloc_pages(1);
    void *p7 = alloc_pages(1);
    void *p8 = alloc_pages(1);
    // void *p4 = alloc_pages(16);
    free_page(p1);
    free_page(p3);
    free_page(p2);
    free_page(p4);
    free_page(p5);
    free_page(p6);
    free_page(p7);
    free_page(p8);
    // free_page_addr(p1);
    // free_page_addr(p2);
    // free_page_addr(p3);
    // _buddy.free(&_buddy, p1);
    // _buddy.free(&_buddy, p2);
    // _buddy.free(&_buddy, p3);
}

void test_mem_pool()
{
    void *p1 = kmalloc(16);
    void *p2 = kmalloc(16);
    void *p3 = kmalloc(32);
    // void *p4 = alloc_pages(16);
    kfree(p1);
    kfree(p3);
    kfree(p2);
}