#include "stdlib.h"
#include "dynamic_alloc.h"
#include "page_alloc.h"
#include "list.h"

extern page_frame_node frame_array[TOTAL_NUM_PAGE];

pool_list pool[33]; // 1, 2, 4, 6, 8, 16, 32
chunk chunk_array[3000];
int global_chunk_index = -1;

void init_pool()
{
    for (int i = 0; i < 33; i++)
        INIT_LIST_HEAD(&pool[i].list);
    return;
}

chunk *new_chunk()
{
    global_chunk_index++;
    chunk_array[global_chunk_index].index = global_chunk_index;
    return &chunk_array[global_chunk_index];
}

void *get_chunk(int req_size)
{
    int req_pool_index = roundup_size(req_size); // req_pool_index * MIN_CHUNK_SIZE = req_size

    if (list_empty(&pool[req_pool_index].list))
    {
        // empty pool on req_size
        int frame_index = get_page_from_free_list(MIN_PAGE_SIZE, req_pool_index);
        split_page(frame_index, req_pool_index);
    }

    int index = remove_a_chunk_from_pool(req_pool_index);
    if (index == -1)
        return NULL;

    void *addr = chunk_array[index].addr;
    return addr;
}

void split_page(int frame_index, int req_pool_index)
{
    int split_size = (req_pool_index * MIN_CHUNK_SIZE);
    for (int i = 0; i < (MIN_PAGE_SIZE / split_size); i++)
    {
        chunk *new = new_chunk();
        new->size = split_size;
        new->addr = (void *)frame_array[frame_index].addr + i *split_size;
        new->val = FREE;
        new->belong_page = frame_index;
        list_add_tail(&new->list, &pool[req_pool_index].list);
    }
}

int remove_a_chunk_from_pool(int req_pool_index)
{
    chunk *alloc_chunk = container_of(pool[req_pool_index].list.next, chunk, list);
    list_del_init(pool[req_pool_index].list.next);
    alloc_chunk->val = ALLOCATED;
    return alloc_chunk->index;
}

void put_back_to_pool(int pool_index, int chunk_index)
{
    // addr to insert
    struct list_head *node = &chunk_array[chunk_index].list;
    unsigned long addr_long = (unsigned long)chunk_array[chunk_index].addr;

    // insert in by the order of addr
    struct list_head *iter = &pool[pool_index].list;
    struct list_head *start = &pool[pool_index].list;
    while (iter->next != start)
    {
        iter = iter->next;
        chunk *tmp = container_of(iter, chunk, list);
        unsigned long iter_addr_long = (unsigned long)tmp->addr;
        if (iter_addr_long > addr_long)
        {
            // list_insert()
            iter->prev->next = node;
            node->prev = iter->prev;
            iter->prev = node;
            node->next = iter;

            tmp->size = -1;
            tmp->val = FREE;

            break;
        }
    }

    // check if there are <MIN_PAGE_SIZE / (pool_index * MIN_CHUNK_SIZE)> free chunks in same page
    iter = &pool[pool_index].list;
    start = &pool[pool_index].list;
    int count = 0;
    int page_id = addr_long >> 12;
    while (iter->next != start)
    {
        iter = iter->next;
        chunk *tmp = list_entry(iter, chunk, list);
        unsigned long tmp_addr_long = (unsigned long)tmp->addr;
        if (tmp_addr_long >> 12 == page_id)
            count++;
        else
            break;
    }
    if (count == (MIN_PAGE_SIZE / (pool_index * MIN_CHUNK_SIZE)))
    {
        // There is a free page
        iter = &pool[pool_index].list;
        start = &pool[pool_index].list;
        while (iter->next != start)
        {
            iter = iter->next;
            chunk *tmp = list_entry(iter, chunk, list);
            unsigned long tmp_addr_long = (unsigned long)tmp->addr;
            if (tmp_addr_long >> 12 == page_id)
                break;
        }
        chunk *first_chunk_in_page = list_entry(iter, chunk, list);
        for (int i = 0; i < count; i++)
        {
            chunk *tmp = list_entry(iter, chunk, list);
            tmp->val = FREE;
            struct list_head *tmp_next = iter->next;
            iter->prev->next = iter->next;
            iter->next->prev = iter->prev;
            iter->prev = iter;
            iter->next = iter;
            iter = tmp_next;
        }
        free_page_frame(first_chunk_in_page->belong_page);
    }

    return;
}

int free_chunk(int index)
{
    // free the page
    int pool_index = chunk_array[index].size / MIN_CHUNK_SIZE;
    put_back_to_pool(pool_index, index);

    return 0;
}

void free(void *addr)
{
    // Check addr is in which page frame
    unsigned long addr_long = (unsigned long)addr;
    int frame_index = (addr_long - FREE_MEM_START) / MIN_PAGE_SIZE;

    if (frame_array[frame_index].val != ALLOCATED)
    {
        printf("This page is Not Allocated yet\n");
        return;
    }

    if (frame_array[frame_index].chunk_order != -1)
    {
        int chunk_index = -1;
        // Find chunk_index
        for (int i = 0; i < global_chunk_index; i++)
        {
            if (addr == chunk_array[i].addr)
            {
                chunk_index = i;
                break;
            }
        }
        // Check if is OK to free
        if (chunk_index >= global_chunk_index || chunk_array[chunk_index].val != ALLOCATED)
        {
            if (chunk_index >= global_chunk_index)
                printf("chunk_index is TOO high\n");
            if (chunk_array[chunk_index].val != ALLOCATED)
                printf("chunk_index = %d\n", chunk_index);
            printf("This chunk is Not Allocated yet\n");
            return;
        }

        free_chunk(chunk_index);
        return;
    }
    else
    {
        free_page_frame(frame_index);
        return;
    }
}

int roundup_size(int size)
{
    switch (size)
    {
    case 1 ... 8:
        return 1;
    case 9 ... 16:
        return 2;
    case 17 ... 32:
        return 4;
    case 33 ... 48:
        return 6;
    case 49 ... 64:
        return 8;
    case 65 ... 128:
        return 16;
    case 129 ... 256:
        return 32;
    }
    return 0;
}

void debug_pool()
{
    printf("** DEBUGGING pool\n");
    for (int i = 0; i < 33; i++)
    {
        struct list_head *iter;
        struct list_head *start;
        iter = &pool[i].list;
        start = &pool[i].list;
        printf("pool[%d] -> ", i);
        while (iter->next != start)
        {
            iter = iter->next;
            chunk *tmp = container_of(iter, chunk, list);
            printf("addr %p -> ", tmp->addr);
        }
        printf("NULL\n");
    }
    printf("**\n");
    printf("** DEBUGGING chunk\n");
    for (int i = 0; i < 20; i++)
    {
        printf("chunk_array[%d].index = %d\n", i, chunk_array[i].index);
        printf("chunk_array[%d].size = %d\n", i, chunk_array[i].size);
        printf("chunk_array[%d].addr = %p\n", i, chunk_array[i].addr);
        printf("chunk_array[%d].val = %d\n", i, chunk_array[i].val);
        printf("chunk_array[%d].belong_page= %d\n", i, chunk_array[i].belong_page);
        printf("\n");
    }
    printf("**\n");
}