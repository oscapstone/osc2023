#include "page_alloc.h"
#include "math.h"
#include "stddef.h"
#include "stdlib.h"
#include "reserve_mem.h"

page_frame_node free_list[MAX_ORDER + 1];
// int frame_array[TOTAL_NUM_PAGE]; // Why NOT use? no malloc to allocate new link list node
page_frame_node frame_array[TOTAL_NUM_PAGE];

// initialize frame_array and free_list
void init_page_frame()
{
    // free_list
    for (int i = 0; i <= MAX_ORDER; i++)
    {
        free_list[i].index = -1; // head of link list
        free_list[i].next = NULL;
        free_list[i].previous = NULL;
    }
    free_list[MAX_ORDER].next = &frame_array[0];

    // frame_array
    frame_array[0].index = 0;
    frame_array[0].val = MAX_ORDER;
    frame_array[0].addr = (void *)FREE_MEM_START;
    frame_array[0].contiguous_head = -1;
    frame_array[0].allocated_order = -1;
    frame_array[0].next = &frame_array[0 + (1 << MAX_ORDER)];
    frame_array[0].previous = &free_list[MAX_ORDER];
    int previous_index = 0;
    for (int i = 1; i < TOTAL_NUM_PAGE; i++)
    {
        frame_array[i].index = i;
        frame_array[i].addr = (void *)FREE_MEM_START + i * MIN_PAGE_SIZE;
        frame_array[i].contiguous_head = -1;
        frame_array[i].allocated_order = -1;
        if (i % (1 << MAX_ORDER) == 0)
        {
            frame_array[i].val = MAX_ORDER;
            frame_array[i].next = NULL;
            frame_array[i].previous = &frame_array[previous_index];
            frame_array[previous_index].next = &frame_array[i];
            previous_index = i;
        }
        else
        {
            frame_array[i].val = FREE_BUDDY;
            frame_array[i].next = NULL;
            frame_array[i].previous = NULL;
        }
    }

    return;
}

int get_page_from_free_list(int req_size, int who)
{
    // int req_num_of_page = -1;
    // if (req_size % MIN_PAGE_SIZE != 0)
    //     req_num_of_page = req_size / MIN_PAGE_SIZE + 1;
    // else
    //     req_num_of_page = req_size / MIN_PAGE_SIZE;

    // calculate req_order
    int req_order = -1;
    for (int i = 0; i <= MAX_ORDER; i++)
    {
        if (req_size <= MIN_PAGE_SIZE * pow(2, i))
        {
            req_order = i;
            break;
        }
    }

    int alloc_index = -1;
    int alloc_order = req_order;
    while (alloc_order <= MAX_ORDER)
    {
        if (free_list[alloc_order].next == NULL) // split high order
        {
            alloc_order++;
        }
        else
            break;
    }
    if (alloc_order > MAX_ORDER)
        return -1;
    while (alloc_order > req_order)
    {
        // split high order
        int removed_index = free_list[alloc_order].next->index;
        remove_from_free_list(free_list[alloc_order].next);
        add_to_free_list(&free_list[alloc_order - 1], removed_index);
        add_to_free_list(&free_list[alloc_order - 1], removed_index + pow(2, alloc_order - 1));
        frame_array[removed_index].val = alloc_order - 1;
        frame_array[removed_index + (1 << (alloc_order - 1))].val = alloc_order - 1;
        alloc_order--;
    }
    if (alloc_order != req_order)
        return -1;

    // get require page
    alloc_index = free_list[alloc_order].next->index;
    remove_from_free_list(free_list[alloc_order].next);
    for (int i = 0; i < (1 << alloc_order); i++)
    {
        frame_array[alloc_index + i].val = ALLOCATED;
        frame_array[alloc_index + i].next = NULL;
        frame_array[alloc_index + i].previous = NULL;
        frame_array[alloc_index + i].contiguous_head = alloc_index;
        frame_array[alloc_index + i].allocated_order = alloc_order;
    }

    // // release redundant page (put back to free_list)
    // int num_of_redundant_page = (1 << alloc_order) - req_num_of_page;
    // if (num_of_redundant_page != 0)
    //     put_back_to_free_list(num_of_redundant_page, alloc_index + req_num_of_page);

    // check the page if contains reserved memory
    unsigned long start = (unsigned long)frame_array[alloc_index].addr;
    unsigned long end = start + MIN_PAGE_SIZE * (1 << req_order);
    int RM_index = check_contain_RM(start, end);
    if (RM_index != 0)
    {
        // Need to change the page allocated
        int new_alloc_index = get_page_from_free_list(req_size, -1);
        free_page_frame(alloc_index);
        alloc_index = new_alloc_index;
    }

#ifdef DEBUG
    debug();
#endif

    frame_array[alloc_index].chunk_order = who;
    return alloc_index;
}

// This does NOT modify frame_array value
void add_to_free_list(page_frame_node *head_node, int index)
{
    page_frame_node *iter = head_node;
    while (iter->next != NULL)
        iter = iter->next;
    iter->next = &frame_array[index];
    frame_array[index].previous = iter;
    frame_array[index].next = NULL;

    return;
}

void remove_from_free_list(page_frame_node *node_to_be_removed)
{
    if (node_to_be_removed->next != NULL)
        node_to_be_removed->next->previous = node_to_be_removed->previous;
    node_to_be_removed->previous->next = node_to_be_removed->next;

    node_to_be_removed->next = NULL;
    node_to_be_removed->previous = NULL;

    return;
}

void put_back_to_free_list(int num_of_redundant_page, int index) // 從 index 開始有 num_of_redundant_page 個 free page
{
    int order_to_put = 0;
    while (num_of_redundant_page >= (1 << order_to_put))
        order_to_put++;
    order_to_put--;
    add_to_free_list(&free_list[order_to_put], index);
    frame_array[index].val = order_to_put;
    if (num_of_redundant_page - (1 << order_to_put) != 0)
        put_back_to_free_list(num_of_redundant_page - (1 << order_to_put), index + (1 << order_to_put));

    return;
}

int free_page_frame(int index)
{
    int contiguous_head = frame_array[index].contiguous_head;
    if (contiguous_head != index)
    {
        printf("Please free the start page of this contiguous memory when allocated, which is index %d\n", contiguous_head);
        return -1;
    }

    // Check if buddy can merge
    int allocated_order = frame_array[index].allocated_order;
    int buddy_index = index ^ (1 << allocated_order);
    if (frame_array[buddy_index].val == allocated_order)
    {
        // can merge
        int merged_order = merge_buddy(index, buddy_index, allocated_order);
        add_to_free_list(&free_list[merged_order], index);
    }
    else
    {
        // can NOT merge
        add_to_free_list(&free_list[allocated_order], index);
        frame_array[index].val = allocated_order;
        frame_array[index].contiguous_head = -1;
        frame_array[index].allocated_order = -1;
        for (int i = 1; i < (1 << allocated_order); i++)
        {
            frame_array[index + i].val = FREE_BUDDY;
            frame_array[index + i].contiguous_head = -1;
            frame_array[index + i].allocated_order = -1;
        }
    }

#ifdef DEBUG
    debug();
#endif

    return 0;
}

// return merged order, YES modify frame_array
int merge_buddy(int index, int buddy, int order)
{
    if (order == MAX_ORDER)
        return order;

    printf("===============index = %d, buddy = %d, order = %d================================\n", index, buddy, order);
    page_frame_node *iter = free_list[order].next;
    while (iter->index != buddy)
    {
        printf("iter->index = %d\n", iter->index);
        iter = iter->next;
    }

    remove_from_free_list(iter);
    frame_array[index].val = order + 1;
    for (int i = 1; i < (1 << (order + 1)); i++)
    {
        frame_array[i].val = FREE_BUDDY;
    }
    int new_buddy = index ^ (1 << (order + 1));

    if (new_buddy < index)
    {
        int tmp = index;
        index = new_buddy;
        new_buddy = index;
    }

    if (frame_array[index].val == frame_array[new_buddy].val)
        return merge_buddy(index, new_buddy, order + 1);
    else
        return order + 1;
}

void debug()
{
    printf("** DEBUGGING free_list\n");
    for (int i = 0; i <= MAX_ORDER; i++)
    {
        page_frame_node *iter;
        iter = &free_list[i];
        printf("free_list[%d] -> ", i);
        while (iter->next != NULL)
        {
            iter = iter->next;
            printf("index %d -> ", iter->index);
        }
        printf("NULL\n");
    }
    printf("**\n");
    printf("** DEBUGGING frame_array\n");
    for (int i = 0; i < 20; i++)
    {
        printf("frame_array[%d].addr = %p\n", i, frame_array[i].addr);
        printf("frame_array[%d].val = %d\n", i, frame_array[i].val);
        printf("frame_array[%d].contiguous_head = %d\n", i, frame_array[i].contiguous_head);
        printf("frame_array[%d].allocated_order = %d\n", i, frame_array[i].allocated_order);
        if (frame_array[i].next != NULL)
            printf("frame_array[%d].next->index = %d\n", i, frame_array[i].next->index);
        if (frame_array[i].previous != NULL)
            printf("frame_array[%d].previous->index = %d\n", i, frame_array[i].previous->index);
    }
    printf("**\n");

    return;
}
