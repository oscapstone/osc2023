#include <allocator.h>

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <utils.h>
#include <uart.h>
#include <devicetree.h>

#define FRAME_SIZE 0x1000
#define LIST_SIZE 17
#define PAGE_NUM 0x3c000000 / FRAME_SIZE
#define F -1
#define X 3

typedef struct NODE_{
    struct NODE_ *next;
    struct NODE_ *prev;
    int index;
    int val;
} NODE;

NODE* free_lists[LIST_SIZE + 1];

NODE node[PAGE_NUM + 1];

extern uint32_t _heap_begin;
extern uint32_t _heap_end;

extern uint32_t _image_memory_begin;
extern uint32_t _image_memory_end;

void *mem_chunk_begin;
void *mem_chunk_end;

int check;

void *simple_malloc(size_t size){
    if(check != 1){
        check = 1;
        mem_chunk_begin = (void *) &_heap_begin;
        mem_chunk_end = (void *) &_heap_end;
    }
    size = (((size - 1) >> 4) + 1) << 4;
    if((uint64_t) mem_chunk_begin + size >= (uint64_t) mem_chunk_end){
        return NULL;
    }
    void *tmp = mem_chunk_begin;
    mem_chunk_begin = (void *)((uint64_t) mem_chunk_begin + size);
    return tmp;
}
///
typedef struct{
    int found;
    char *initrd_start;
    char *initrd_end;
} ramdisk_data;

int malloc_ramdisk_fdt_callback(char* node, const char *name, void *data){
    if(strcmp(name, "chosen")) return 0;
    ramdisk_data* _data = (ramdisk_data*) data;
    fdt_prop *prop;
    while(prop = fdt_nextprop(node + 1, &node)){
        if(!strcmp(prop->name,"linux,initrd-start")){
            _data->initrd_start = (char *)(uint64_t)ntohl(*(uint32_t*)prop->value);
        }
        if(!strcmp(prop->name,"linux,initrd-end")){
            _data->initrd_end = (char *)(uint64_t)ntohl(*(uint32_t*)prop->value);
        }
    }
    _data->found = 1;
    return 1;
}

int ramdisk_reserve(){
    ramdisk_data data;
    data.initrd_start = 0;
    data.initrd_end = 0;
    data.found = 0;
    fdt_traverse(malloc_ramdisk_fdt_callback, (void *)&data);
    if(data.found){
        memory_reserve((uint64_t)data.initrd_start, (uint64_t)data.initrd_end);
        return 0;
    }
    return -1;
}
///

int log_2(uint64_t num){
    int count = 0;
    uint64_t tmp = 1;
    while(tmp < num){
        count++;
        tmp <<= 1;
    }
    return count;
}

uint64_t phy_addr(NODE* tmp){
    return (uint64_t) (tmp->index * FRAME_SIZE);
}

uint64_t block_size(NODE* tmp){
    return (uint64_t) (FRAME_SIZE * (1 << tmp->val));
}

void show_node_info(NODE* tmp){
    int i = tmp->index;
    uart_print("Node[");
    uart_print_hex(i, 32);
    uart_print("]: addr(");
    uart_print_hex(phy_addr(&node[i]), 64);
    uart_print("), size(");
    uart_print_hex(block_size(&node[i]), 64);
    uart_print(")");
    newline();
}

void list_erase(int i){
    uart_print("Erase node:\n");
    show_node_info(&node[i]);
    int k = node[i].val;
    if(free_lists[k] && free_lists[k]->index == i){
        free_lists[k] = free_lists[k]->next;
        if(free_lists[k]) free_lists[k]->prev = NULL;
        node[i].prev = NULL;
        node[i].next = NULL;
        node[i].val = -2;
        return;
    }else{
        NODE *now = &node[i];
        now->prev->next = now->next;
        if(now->next) now->next->prev = now->prev;
        node[i].prev = NULL;
        node[i].next = NULL;
        node[i].val = -2;
        return;
    }
    uart_print("Erase error!!!\n");
}

void list_insert(int i, NODE* tmp){
    if(free_lists[i] == NULL){
        free_lists[i] = tmp;
    }else{
        tmp->next = free_lists[i];
        free_lists[i]->prev = tmp;
        free_lists[i] = tmp;
    }
    uart_print("list insert ");
    uart_print_hex(i, 32);
    newline();
    show_node_info(tmp);
}

NODE *list_top(int i){
    return free_lists[i];
}

void buddy_system_init() {
    uart_print("Allocator init\n");
    for(int i = 0; i <= PAGE_NUM; i++){
        node[i].next = NULL;
        node[i].prev = NULL;
        node[i].index = i;
        if(i == 0x00){
            node[i].val = log_2(0x20000);
            list_insert(node[i].val, &node[i]);
        }else if(i == 0x20000){
            node[i].val = log_2(0x10000);
            list_insert(node[i].val, &node[i]);
        }else if(i == 0x30000){
            node[i].val = log_2(0x8000);
            list_insert(node[i].val, &node[i]);
        }else if(i == 0x38000){
            node[i].val = log_2(0x4000);
            list_insert(node[i].val, &node[i]);
        }else{
            node[i].val = F;
        }
    }
}

void malloc_init(){
    for(int i = 0; i < LIST_SIZE + 1; i++){
        free_lists[i] = NULL;
    }
    
    buddy_system_init();
    memory_reserve(0x0000, 0x1000); // 1
    memory_reserve((uint64_t) &_image_memory_begin, (uint64_t) &_image_memory_end);// 2, 5
    ramdisk_reserve(); // 3
    if((uint64_t) _devicetree_begin != 0xffffffff){ // 4
        fdt_header *header = (fdt_header *) _devicetree_begin;
        uint64_t _devicetree_end = (uint64_t) _devicetree_begin + ntohl(header->totalsize);
        memory_reserve((uint64_t)_devicetree_begin, (uint64_t) _devicetree_end);
    }
}

int node_split(NODE* tmp, int power_size){
    if(power_size <= 0) return -1;
    int r = tmp->index ^ (1 << (power_size - 1));
    list_erase(tmp->index);
    tmp->val = power_size - 1;
    node[r].val = power_size - 1;
    uart_print("Split buddy: \n");
    uart_print("1: \n");
    show_node_info(tmp);
    uart_print("2: \n");
    show_node_info(&node[r]);
    list_insert(power_size - 1, &node[r]);
    list_insert(power_size - 1, tmp);
    return r;
}

void *buddy_system_malloc(size_t size){
    uart_print("-------------\n");
    uart_print("Request size: ");
    uart_print_hex(size, 32);
    newline();
    size = size / FRAME_SIZE + (size % FRAME_SIZE != 0);
    const int o_power_size = log_2(size);
    int power_size = o_power_size;
    while(power_size <= LIST_SIZE){
        if(free_lists[power_size] == NULL){
            power_size++;
            continue;
        }
        if(free_lists[power_size]->val < 0){
            uart_print_hex(free_lists[power_size]->val, 32);
            uart_print("alloc error!!!\n");
        }
        NODE *tmp = list_top(power_size);
        while(o_power_size < power_size){
            node_split(tmp, power_size);
            power_size--;
        }
        uart_print("Found node: ");
        show_node_info(tmp);
        int tmp_val = tmp->val;
        list_erase(tmp->index);
        uart_print("-------------\n");
        
        uint64_t addr_tmp = phy_addr(tmp);
        tmp->val = -1 * tmp_val - X;
        return (void *) addr_tmp;
    }
    uart_print("No enough space!!!\n");
    uart_print("-------------\n");
    return NULL;
}



void node_merge(NODE* tmp){
    if(tmp->index == 0x0 && tmp->val == log_2(0x20000)) return;
    if(tmp->index == 0x20000 && tmp->val == log_2(0x10000)) return;
    if(tmp->index == 0x30000 && tmp->val == log_2(0x8000)) return;
    if(tmp->index == 0x38000 && tmp->val == log_2(0x4000)) return;
    int r = tmp->index ^ (1 << tmp->val);
    if(node[r].val == tmp->val){
        uart_print("Merge node: \n");
        uart_print("1: \n");
        show_node_info(tmp);
        uart_print("2: \n");
        show_node_info(&node[r]);
        int tmp_val = tmp->val;
        list_erase(tmp->index);
        list_erase(r);
        if(tmp->index > r){
            node[r].val = tmp_val + 1;
            tmp->val = -1;
            list_insert(node[r].val, &node[r]);
            node_merge(&node[r]);
        }else{
            node[r].val = -1;
            tmp->val = tmp_val + 1;
            list_insert(tmp->val, tmp);
            node_merge(tmp);
        }
    }
}

void buddy_system_free(void *tmp){
    uart_print("-------------\n");
    uart_print("Free node: ");
    uart_print_hex((uint64_t) tmp, 64);
    newline();
    uint64_t i = (uint64_t) tmp / FRAME_SIZE;
    if(node[i].val > -1 * X){
        uart_print("free error!\n");
    }else{
        node[i].val = (node[i].val + X) * -1;
        list_insert(node[i].val, &node[i]);
        node_merge(&node[i]);
    }
    uart_print("-------------\n");
}

void __memory_reserve(int i, uint64_t begin, uint64_t end){
    uart_print("Check ");
    uart_print_hex(i, 64);
    newline();
    int lbound = phy_addr(&node[i]);
    int rbound = phy_addr(&node[i + (1 << node[i].val)]);
    if(begin <= lbound && end >= rbound){
        list_erase(i);
    }else if(end <= lbound || begin >= rbound){
        return;
    }else{
        int j = node_split(&node[i], node[i].val);
        if(j == -1) {
            list_erase(i);
            return;
        }
        __memory_reserve(i, begin, end);
        __memory_reserve(j, begin, end);
    }
}

void memory_reserve(uint64_t begin, uint64_t end) {
    uart_print("Reserve ");
    uart_print_hex(begin, 64);
    uart_print(" - ");
    uart_print_hex(end, 64);
    newline();
    for(int i = 0; i <= LIST_SIZE; i++){
        for(NODE * now = free_lists[i]; now != NULL; now = now->next){
            __memory_reserve(now->index, begin, end);
        }
    }
}

typedef struct per_chunk_{
    struct per_chunk_ *next;
    uint64_t addr;
} per_chunk;

typedef struct chunk_{
    struct chunk_ *t_next;
    per_chunk * next;
    uint64_t begin;
    uint64_t end;
} chunk;

chunk *chunk_table = NULL;

void *kmalloc(size_t size){
    if(chunk_table == NULL){
        chunk_table = simple_malloc(sizeof(chunk));
        chunk_table->begin = (uint64_t) buddy_system_malloc(0x1000);
        chunk_table->end = chunk_table->begin + 0x1000;
        chunk_table->t_next = NULL;
        chunk_table->next = NULL;
    }
    chunk *now = chunk_table;
    size = (((size - 1) >> 4) + 1) << 4;
    while((uint64_t) now->begin + size >= (uint64_t) now->end){
        if(now->t_next){
            now = now->t_next;
            continue;
        }else{
            chunk* chunk_tmp = now->t_next;
            now->t_next = simple_malloc(sizeof(chunk));
            now = now->t_next;
            now->begin = (uint64_t) buddy_system_malloc(0x1000);
            now->end = now->begin + 0x1000;
            now->t_next = chunk_tmp;
            now->next = NULL;
        }
    }
    uint64_t tmp = now->begin;
    now->begin = ((uint64_t) now->begin + size);
    if(now->next == NULL){
        now->next = simple_malloc(sizeof(per_chunk));
        now->next->addr = tmp;
        now->next->next = NULL;
    }else{
        per_chunk *per_chunk_tmp = now->next;
        now->next = simple_malloc(sizeof(per_chunk));
        now->next->addr = tmp;
        now->next->next = per_chunk_tmp;
        now->next->next = NULL;
    }
    
    return (void *)tmp;
}

void kfree(void * tmp){
    chunk *now = chunk_table;
    chunk *prev = chunk_table;
    for(; now != NULL; now = now->t_next){
        if(now->next->addr == (uint64_t) tmp){
            now->next = now->next->next;
            if(now->next == NULL){
                buddy_system_free((void * )now->begin);
                if(chunk_table == now){
                    chunk_table = NULL;
                }else{
                    prev->t_next = now->t_next;
                }
            }
            return;
        }
        for(per_chunk* cur = now->next; cur != NULL; cur = cur->next){
            if((uint64_t) tmp == cur->next->addr){
                cur->next = cur->next->next;
                return;
            }
        }
        prev = now;
    }
}