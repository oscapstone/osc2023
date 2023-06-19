#include<mm.h>
#include<dt17.h>
#include<string.h>
#include<page_alloc.h>
#include<sc_alloc.h>
#include<boot.h>
#include<dt17.h>
#include<mem.h>
#include<syscall.h>
#include<utils.h>
#include<mini_uart.h>

extern char _stack_top;

static uint64 memory_end;
static uint32 memory_node;

static uint64 ld64(char *start)
{
    uint64 ret = 0;
    uint32 i = 8;

    while (i--) {
        ret <<= 8;
        ret += (uint8)(*start++);
    }

    return ret;
}

static int memory_fdt_parser(int level, char *cur, char *dt_strings){
    fdt32_t tag = fdt32_ld((fdt32_t*)cur);
    switch(tag){
        case FDT_BEGIN_NODE:
            cur+=sizeof(fdt32_t);
            if(!strcmp("memory@0", cur)){
                memory_node = 1;
                // uart_printf("find memory node!\r\n");
            }
            else
                memory_node = 0;
            break;
        case FDT_PROP:
            if(!memory_node)
                break;           
            cur += sizeof(fdt32_t);
            struct fdt_property *fdtp = (struct fdt_property *)cur;
            // uart_printf("dt_string: %s\r\n", dt_strings + fdtp_nameoff(fdtp));
            if(!strcmp("reg",dt_strings + fdtp_nameoff(fdtp))){
                cur += sizeof(struct fdt_property);
                memory_end = ld64(cur);

                // find the memory_end addr, return immediately.
                uart_printf("[*] memory_end: %x\r\n", memory_end);
                return 1;
            }
            break;
        case FDT_END_NODE:
            memory_node = 0;
            break;
        case FDT_NOP:
        case FDT_END:
            break;
    }
    return 0;
}

void mm_init(char *fdt_base){
    parse_dtb(fdt_base, memory_fdt_parser);

    if(!memory_end)
        uart_printf("[x] Cannot find memory end addr in fdt.\r\n");
    page_allocator_early_init((void *)PA2VA(0),(void *)PA2VA(memory_end));
    sc_early_init();

    // Spin tables for multicore boot
    mem_reserve((void *)PA2VA(0), (void *)PA2VA(0x4000));

    // Kernel image in the physical memory
    mem_reserve(_start, (void *)&_stack_top);

    // Initramfs
    mem_reserve(_initramfs_addr, _initramfs_end);

    // Simple malloc
    mem_reserve(SMEM, EMEM);

    page_allocator_init();
    sc_init();

#ifdef DEBUG
    page_allocator_test();
    sc_test();
#endif

}

void *kmalloc(int size){
    uint32 daif;
    void *ret;

    daif = save_and_disable_interrupt();

    if (size <= PAGE_SIZE) {
        // Use the Small Chunk allocator
        ret = sc_alloc(size);
    } else {
        // Use the Buddy System allocator
        int page_cnt = ALIGN(size, PAGE_SIZE) / PAGE_SIZE;

        ret = alloc_pages(page_cnt);
    }

    restore_interrupt(daif);

    return ret;
}

void kfree(void *ptr)
{
    uint32 daif;

    daif = save_and_disable_interrupt();

    if (!sc_free(ptr)) {
        /*
         * The chunk pointed to by ptr is managed by the Small Chunk
         * allocator.
         */
        goto _KFREE_END;
    }

    free_page(ptr);

_KFREE_END:

    restore_interrupt(daif);
}