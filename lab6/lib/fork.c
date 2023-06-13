#include "fork.h"
#include "mm.h"
#include "mini_uart.h"
#include "../include/sched.h"
#include "../include/entry.h"
#include "mmu.h"

int copy_process(unsigned long clone_flags, unsigned long fn, unsigned long arg/*, unsigned long stack*/) {
    
    preempt_disable();
    struct task_struct *p;

    p = (struct task_struct *) allocate_kernel_page();
    if (p == NULL)
        return -1;

    memzero((unsigned long)p, sizeof(struct task_struct));
    struct pt_regs *childregs = task_pt_regs(p);

    if (clone_flags & PF_KTHREAD) {
        p->cpu_context.x19 = fn;
        p->cpu_context.x20 = arg;
    } else {
        struct pt_regs *cur_regs = task_pt_regs(current);
        // *childregs = *cur_regs; (object file generates memcpy)
        // therefore the for loop is used below
        for(int i=0; i<sizeof(struct pt_regs); i++) {
            ((char*)childregs)[i] = ((char*)cur_regs)[i];
        }
        childregs->regs[0] = 0; // return value 0
        copy_virt_memory(p);
    }

    p->flags = clone_flags;
    p->priority = current->priority;
    p->state = TASK_RUNNING;
    p->counter = p->priority;
    p->preempt_count = 1;
    
    p->cpu_context.pc = (unsigned long)ret_from_fork;
    p->cpu_context.sp = (unsigned long)childregs;

    int pid = nr_tasks++;
    task[pid] = p;
    p->id = pid;
    preempt_enable();

    return pid;

}

int move_to_user_mode(unsigned long start, unsigned long size, unsigned long pc) {
    
    struct pt_regs *regs = task_pt_regs(current);
    regs->pc = pc; // virt pc
    regs->pstate = PSR_MODE_EL0t;
    regs->sp = 0xfffffffff000;

    unsigned long pages = (unsigned long)malloc(size); // phys 
    if (pages == NULL)
        return -1;

    unsigned long va; // might need to map more? bss? currently mapping just enough
    for(va=0; va<size; va+=PAGE_SIZE) {
        map_page(current, va, pages+va);
    }

    unsigned long stack_bot = (unsigned long)malloc(PAGE_SIZE*4); // phys
    if (stack_bot == NULL)
        return -1;

    unsigned int i=0;    
    for(va=0xffffffffb000; va<regs->sp; va+=PAGE_SIZE) {
        map_page(current, va, stack_bot+(i*PAGE_SIZE));
        i++;
    }

    // vc identity mapping
    //printf("user page count %d\n", current->mm.user_pages_count);
    for(unsigned long va=0x3c000000; va<0x3f000000; va+=PAGE_SIZE) {
        unsigned long pgd;
        if (!current->mm.pgd) {
            pgd = (unsigned long)malloc(PAGE_SIZE);
            //printf("pgd not found, created at phys address 0x%x\n", pgd);
            memzero(pgd+VA_START, PAGE_SIZE);
            current->mm.pgd = pgd;
            current->mm.kernel_pages[++current->mm.kernel_pages_count] = current->mm.pgd;
        }
        pgd = current->mm.pgd;
        //printf("pgd at 0x%x\n", pgd);
        int new_table;
        unsigned long pud = map_table((unsigned long *)(pgd + VA_START), PGD_SHIFT, va, &new_table);
        //printf("pud at 0x%x\n", pud);
        if (new_table) {
            current->mm.kernel_pages[++current->mm.kernel_pages_count] = pud;
        }
        unsigned long pmd = map_table((unsigned long *)(pud + VA_START), PUD_SHIFT, va, &new_table);
        //printf("pmd at 0x%x\n", pmd);
        if (new_table) {
            current->mm.kernel_pages[++current->mm.kernel_pages_count] = pmd;
        }
        unsigned long pte = map_table((unsigned long *)(pmd + VA_START), PMD_SHIFT, va, &new_table);
        //printf("pte at 0x%x\n", pte);
        if (new_table) {
            current->mm.kernel_pages[++current->mm.kernel_pages_count] = pte;
        }
        map_table_entry((unsigned long *)(pte + VA_START), va, va);
        //struct user_page p = {page, va};
        //task->mm.user_pages[task->mm.user_pages_count++] = p;
        //if (va == 0x3c25e000) printf("0x%x, 0x%x, 0x%x, 0x%x\n", pgd, pud, pmd, pte);
    }
    //printf("user page count %d\n", current->mm.user_pages_count);
    //memcpy(pages+VA_START, start, size); // move code to pages
    for(int i=0; i<size; i++)
        ((char*)pages+VA_START)[i] = ((char*)start)[i];
    update_pgd(current->mm.pgd);
    return 0;

}

struct pt_regs *task_pt_regs(struct task_struct *tsk) {

    unsigned long p = (unsigned long)tsk + THREAD_SIZE - sizeof(struct pt_regs);
    return (struct pt_regs *)p;

}

void new_user_process(unsigned long func){
	printf("Kernel process started, moving to user mode.\n");
	int err = move_to_user_mode(func, 4096, func);
	if (err < 0){
		printf("Error while moving process to user mode\n\r");
	} 
}

int copy_virt_memory(struct task_struct *dst) {
    struct task_struct* src = current;
    for (int i=0; i<src->mm.user_pages_count; i++) {
        unsigned long kernel_va = allocate_user_page(dst, src->mm.user_pages[i].virt_addr);
        if(kernel_va == 0) return -1;
        memcpy(kernel_va, src->mm.user_pages[i].virt_addr, PAGE_SIZE);
    }
    for(unsigned long va=0x3c000000; va<0x3f000000; va+=PAGE_SIZE) {
        unsigned long pgd;
        if (!dst->mm.pgd) {
            pgd = (unsigned long)malloc(PAGE_SIZE);
            //printf("pgd not found, created at phys address 0x%x\n", pgd);
            memzero(pgd+VA_START, PAGE_SIZE);
            dst->mm.pgd = pgd;
            dst->mm.kernel_pages[++dst->mm.kernel_pages_count] = dst->mm.pgd;
        }
        pgd = dst->mm.pgd;
        //printf("pgd at 0x%x\n", pgd);
        int new_table;
        unsigned long pud = map_table((unsigned long *)(pgd + VA_START), PGD_SHIFT, va, &new_table);
        //printf("pud at 0x%x\n", pud);
        if (new_table) {
            dst->mm.kernel_pages[++dst->mm.kernel_pages_count] = pud;
        }
        unsigned long pmd = map_table((unsigned long *)(pud + VA_START), PUD_SHIFT, va, &new_table);
        //printf("pmd at 0x%x\n", pmd);
        if (new_table) {
            dst->mm.kernel_pages[++dst->mm.kernel_pages_count] = pmd;
        }
        unsigned long pte = map_table((unsigned long *)(pmd + VA_START), PMD_SHIFT, va, &new_table);
        //printf("pte at 0x%x\n", pte);
        if (new_table) {
            dst->mm.kernel_pages[++dst->mm.kernel_pages_count] = pte;
        }
        map_table_entry((unsigned long *)(pte + VA_START), va, va);
        //struct user_page p = {page, va};
        //task->mm.user_pages[task->mm.user_pages_count++] = p;
        if (va == 0x3c25e000) printf("0x%x, 0x%x, 0x%x, 0x%x\n", pgd, pud, pmd, pte);
    }
    return 0;
}