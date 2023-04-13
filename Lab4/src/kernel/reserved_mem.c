#include "reserve_mem.h"
#include "string.h"

reserved_memory_block RMarray[100];
int RMindex = 0;

void memory_reserve(int start, int end, char *name)
{
    RMindex++;
    RMarray[RMindex].start = start;
    RMarray[RMindex].end = end;
    strcpy(name, RMarray[RMindex].name);
}