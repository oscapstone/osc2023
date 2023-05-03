#ifndef _RESERVE_MEM_H
#define _RESERVE_MEM_H

#define USRPGM_BASE 0x15000000
#define USRPGM_SIZE 0x100000

typedef struct _reserved_memory_block
{
    unsigned long start;
    unsigned long end;
    char name[30];
} reserved_memory_block;

void memory_reserve(unsigned long start, unsigned long end, char *name);
int check_contain_RM(unsigned long start, unsigned long end);
void memory_init();

#endif /*_RESERVE_MEM_H */