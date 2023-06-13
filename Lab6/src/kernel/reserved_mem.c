#include "reserve_mem.h"
#include "page_alloc.h"
#include "dynamic_alloc.h"
#include "reserve_mem.h"
#include "stdlib.h"
#include "virtual_mem.h"

reserved_memory_block RMarray[100];
int RMindex = 0;

void memory_reserve(unsigned long start, unsigned long end, char *name)
{
    RMindex++;
    RMarray[RMindex].start = start;
    RMarray[RMindex].end = end;
    strcpy(RMarray[RMindex].name, name);
}

// return value : if including RM, return which no. of RM. Otherwise, return 0.
int check_contain_RM(unsigned long start, unsigned long end)
{
    for (int i = 1; i <= RMindex; i++)
    {
        if (RMarray[i].start <= start && start <= RMarray[i].end)
            return i;
        else if (RMarray[i].start <= end && end <= RMarray[i].end)
            return i;
        else if (start <= RMarray[i].start && RMarray[i].end <= end)
            return i;
        else
            continue;
    }
    return 0;
}

void memory_init()
{
    init_page_frame();
    init_pool();

    memory_reserve(KERNEL_PA_TO_VA(0x0000), KERNEL_PA_TO_VA(0x4000), "kernel PGD, PUD");
    memory_reserve(KERNEL_PA_TO_VA(0x60000), KERNEL_PA_TO_VA(0x100000), "Kernel Img");
    memory_reserve(KERNEL_PA_TO_VA(0x1000000), KERNEL_PA_TO_VA(0x1000fff), "Printf Buffer");
    memory_reserve(KERNEL_PA_TO_VA(0x8000000), KERNEL_PA_TO_VA(0x8010000), "Initramfs");
    memory_reserve(KERNEL_PA_TO_VA(0x15000000), KERNEL_PA_TO_VA(0x17000000), "User Program");
    memory_reserve(KERNEL_PA_TO_VA(0x200000), KERNEL_PA_TO_VA(0x250000), "svc");

    return;
}