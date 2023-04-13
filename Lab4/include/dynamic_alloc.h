#ifndef _DYNAMIC_ALLOC_H
#define _DYNAMIC_ALLOC_H

#include "list.h"

#define MAX_POOL_SIZE 256
#define MIN_CHUNK_SIZE 8
#define FREE 98

typedef struct _chunk
{
    int index; // const
    int size; 
    void *addr; // const
    int val;
    int belong_page;
    struct list_head list;
} chunk;

typedef struct _pool_list
{
    struct list_head list;
} pool_list;

void init_pool();
int get_chunk(int req_size);
int roundup_size(int size);
void split_page(int frame_index, int req_pool_index);
int remove_a_chunk_from_pool(int req_pool_index);
int free_chunk(int index);
void put_back_to_pool(int pool_index, int chunk_index);
void debug_pool();

#endif /*_DYNAMIC_ALLOC_H */
