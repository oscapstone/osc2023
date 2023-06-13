#include "mmu.h"
#include "malloc.h"
#include "string.h"
#include "threads.h"
#include "exception.h"
// PGD's page frame at 0x1000
// PUD's page frame at 0x2000
// two-level translation (1G) -> three-level translation (2MB)
void* set_2M_kernel_mmu(void* x0)
{
    //unsigned long *pud_table = kmalloc(0x1000);
    unsigned long *pud_table = (unsigned long *)0x3000;
    for (int i = 0; i < 512; i++)
    {
        // 0x3F000000 to 0x3FFFFFFF for peripherals
        if ((0x00000000 + (0x200000L) * i) >= 0x3F000000L)
        {
            pud_table[i] = PD_ACCESS | PD_BLOCK | (0x00000000 + (0x200000L) * i) | (MAIR_DEVICE_nGnRnE << 2) | PD_UK_ACCESS | PD_UNX | PD_KNX;
        }
        else
        {
            pud_table[i] = PD_ACCESS | PD_BLOCK | (0x00000000 | (0x200000L) * i) | (MAIR_IDX_NORMAL_NOCACHE << 2);
        }
    }

    //unsigned long *pud_table2 = kmalloc(0x1000);
    unsigned long *pud_table2 = (unsigned long *)0x4000;
    for (int i = 0; i < 512; i++)
    {
        pud_table2[i] = PD_ACCESS | PD_BLOCK | (0x40000000L | (0x200000L) * i) | (MAIR_IDX_NORMAL_NOCACHE << 2);
    }

    // set PUD
    *(unsigned long *)(kernel_pud_addr) = PD_ACCESS | PD_TABLE | (unsigned long)pud_table;
    *(unsigned long *)(kernel_pud_addr + 0x8) = PD_ACCESS  | PD_TABLE | (unsigned long)pud_table2;

    return x0;
}

// pa,va aligned to 4K
void map_one_page(size_t *virt_pgd_p, size_t va, size_t pa, size_t flag)
{
    size_t *table_p = virt_pgd_p;
    for (int level = 0; level < 4; level++)
    {
        unsigned int idx = (va >> (39 - level * 9)) & 0x1ff;

        if (level == 3)
        {
            table_p[idx] = pa;
            table_p[idx] |= PD_ACCESS | PD_TABLE | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_KNX | flag;
            return;
        }

        if(!table_p[idx])
        {
            size_t* newtable_p =kmalloc(0x1000);
            memset(newtable_p, 0, 0x1000);
            table_p[idx] = VIRT_TO_PHYS((size_t)newtable_p);
            table_p[idx] |= PD_ACCESS | PD_TABLE | (MAIR_IDX_NORMAL_NOCACHE << 2);
        }

        table_p = (size_t*)PHYS_TO_VIRT((size_t)(table_p[idx] & ENTRY_ADDR_MASK));
    }
}

//give 
void mappages(size_t *virt_pgd_p, size_t va, size_t size, size_t pa, size_t flag)
{
    pa = pa - (pa % 0x1000); // align
    for (size_t s = 0; s < size; s+=0x1000)
    {
        map_one_page(virt_pgd_p, va + s, pa + s, flag);
    }
}

void add_vma(thread_t *t, size_t va, size_t size, size_t pa, size_t rwx, int is_alloced)
{
    // alignment
    size = size % 0x1000 ? size + (0x1000 - size%0x1000): size;
    
    vm_area_struct_t *new_area = kmalloc(sizeof(vm_area_struct_t));
    new_area->rwx = rwx;
    new_area->area_size = size;
    new_area->virt_addr = va;
    new_area->phys_addr = pa;
    new_area->is_alloced = is_alloced;
    list_add_tail((list_head_t *)new_area, &t->vma_list);
}

void free_page_tables(size_t *page_table, int level)
{
    size_t *table_virt = (size_t*)PHYS_TO_VIRT((char*)page_table);
    for (int i = 0; i < 512; i++)
    {
        if (table_virt[i] != 0)
        {
            size_t *next_table = (size_t*)(table_virt[i] & ENTRY_ADDR_MASK);
            if (table_virt[i] & PD_TABLE)
            { 
                if(level!=2)free_page_tables(next_table, level + 1);
                table_virt[i] = 0L;
                kfree(PHYS_TO_VIRT((char *)next_table));
            }
        }
    }
}

void handle_abort(esr_el1_t* esr_el1)
{
    unsigned long long far_el1;
    __asm__ __volatile__("mrs %0, FAR_EL1\n\t": "=r"(far_el1));

    list_head_t *pos;
    vm_area_struct_t *the_area_ptr = 0;
    list_for_each(pos, &curr_thread->vma_list)
    {
        if (((vm_area_struct_t *)pos)->virt_addr <= far_el1 && ((vm_area_struct_t *)pos)->virt_addr + ((vm_area_struct_t *)pos)->area_size >= far_el1)
        {
            the_area_ptr = (vm_area_struct_t *)pos;
            break;
        }
    }

    if (!the_area_ptr)
    {
        seg_fault();
    }

    // For translation fault
    if ((esr_el1->iss & 0x3f) == TF_LEVEL0 || (esr_el1->iss & 0x3f) == TF_LEVEL1 || (esr_el1->iss & 0x3f) == TF_LEVEL2 || (esr_el1->iss & 0x3f) == TF_LEVEL3)
    {
        //uart_printf("[Translation fault]: 0x%x\r\n",far_el1);

        size_t addr_offset = (far_el1 - the_area_ptr->virt_addr);
        addr_offset = (addr_offset % 0x1000) == 0 ? addr_offset : addr_offset - (addr_offset % 0x1000);
        map_one_page_rwx(PHYS_TO_VIRT(curr_thread->context.ttbr0_el1), the_area_ptr->virt_addr + addr_offset, the_area_ptr->phys_addr + addr_offset, the_area_ptr->rwx);
    }else
    {
        // For other Fault (permisson ...etc)
        seg_fault();
    }
}

void map_one_page_rwx(size_t *pgd_p, size_t va, size_t pa, size_t rwxflag)
{
    size_t flag = 0;

    //execute
    if(!(rwxflag & 0b100))flag |= PD_UNX;

    //write
    if(!(rwxflag & 0b10))flag |= PD_RDONLY;

    //read
    if (rwxflag & 0b1)flag |= PD_UK_ACCESS;

    map_one_page(pgd_p, va, pa, flag);
}

void seg_fault()
{
    uart_printf("[Segmentation fault]: Kill Process\r\n");
    thread_exit();
}