#ifndef __MEM_H
#define __MEM_H
#include "type.h"

#ifndef NULL
    #define NULL (0)
#endif

void *simple_malloc(uint32_t size);
void simple_free(void *ptr);
void smem_init();
void *kmalloc(uint32_t size);
void *cmalloc(uint32_t size);
void kfree(void *ptr);
void set_init_mem_region(char *name, char *prop_name, char *data);

#endif