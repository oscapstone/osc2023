#include "str.h"
#include "mmu.h"
#include "vm.h"
#include "uart.h"
#include "mem.h"
#include <stdint.h>
#include <stddef.h>

/**************************************************************************
 * Translate physical memroy address to virtual address
 *
 * @addr: input physical address
 *
 * This is only for the kernel memory (ttbr1_el1)
 *************************************************************************/
uint64_t phy2vir(void* addr){
	uint64_t tmp = (0xffff000000000000 | (uint64_t)addr);
	//uart_puthl(tmp);
	//return (0xffff000000000000 | (uint64_t)addr);
	return tmp;
}

uint64_t vir2phy(void* addr){
	return (0xffffffff & (uint64_t)addr);
}

/************************************************************************
 * Helper function which create a page table.
 ***********************************************************************/
static uint64_t page_table_init(){
	uint64_t t = pmalloc(0);
	memset(phy2vir(t), 0, 0x1000);
	/*
	uart_puts("init...");
	uart_puthl(t);
	uart_puts("\n");
	*/
	//t = phy2vir(t);		// NOTE: this addr should be kernel addr
	return (t | BOOT_PGD_ATTR);
}

/*************************************************************************
 * Mapping physical memory to physical memory
 *
 * @pt: pointer to PGD (L0 page table)
 * @vm: virtual address want to map to
 * @pm: physical address want to map from
 * @len: how many page want to map to (4KB)
 ************************************************************************/
int map_vm(uint64_t* pt, uint64_t vm, uint64_t pm, int len){
	if(len <= 0)
		return 1;
	if(pt == NULL)
		return 1;

	pt = phy2vir(pt);
	//uart_puthl(pt);
	int pud, pmd, pte, pgd;
	uint64_t *ptr_pte, *ptr_pmd, *ptr_pud, *ptr_pgd;
	for(int i = 0; i < len; i++){
		uint64_t tmp_vm = vm + i * 0x1000;
		pte = (tmp_vm >> 12) & 0x1ff;
		pmd = (tmp_vm >> 21) & 0x1ff;
		pud = (tmp_vm >> 30) & 0x1ff;
		pgd = (tmp_vm >> 39) & 0x1ff;
		/*
		uart_puts("pte\n ");
		uart_puthl(pte);
		uart_puts("pmd \n");
		uart_puthl(pmd);
		uart_puts("pud\n");
		uart_puthl(pud);
		uart_puts("pgd \n");
		uart_puthl(pgd);
		*/
		
		// Check if each level page table exist
		if(pt[pgd] == 0){
			pt[pgd] = page_table_init();
			//uart_puts("pt[pgd] init ");
		}
		ptr_pgd = pt[pgd] & 0xfffffffffffff000;
		ptr_pgd = phy2vir(ptr_pgd);
		/*
		uart_puts("\n pt[pgd]: ");
		uart_puti(pgd);
		uart_puthl(pt[pgd]);
		*/
		if(ptr_pgd[pud] == 0){
			ptr_pgd[pud] = page_table_init();
		}
		ptr_pud = ptr_pgd[pud] & 0xfffffffffffff000;
		ptr_pud = phy2vir(ptr_pud);
		//ptr_pud[0] = 0x0 |  BOOT_NORMAL_ATTR;
		if(ptr_pud[pmd] == 0){
			ptr_pud[pmd] = page_table_init();
		}
		ptr_pmd = ptr_pud[pmd] & 0xfffffffffffff000;
		ptr_pmd = phy2vir(ptr_pmd);

		if(ptr_pmd[pte] != 0){
			uart_puts("the block has been mapped!\n");
			/*
			uart_puthl(ptr_pmd[pte]);
			uart_puthl(pm + i * 0x1000);
			*/
			return 1;
		}

		//Write the record
		ptr_pmd[pte] = (((pm + i * 0x1000) | PAGE_NORMAL_ATTR) &0xffffffff);
		/*
		uart_puts("\n pte value: ");
		uart_puthl(ptr_pmd[pte]);
		*/
	}
	return 0;
}

int copy_vm(uint64_t* from, uint64_t*  to){
	from = phy2vir(from);
	to = phy2vir(to);
	uint64_t *ptr_pte, *ptr_pmd, *ptr_pud, *ptr_pgd;
	uint64_t *to_ptr_pte, *to_ptr_pmd, *to_ptr_pud, *to_ptr_pgd;
	// Get pud
	// 511 to pass the stack copy
	for(int i = 0; i < 511; i ++){
		if(from[i] == NULL )
			continue;
		ptr_pgd = phy2vir(from[i] & 0xfffffffffffff000); 
		if(to[i] == NULL) 
			to[i] = page_table_init(); 
		to_ptr_pgd = phy2vir(to[i] & 0xfffffffffffff000); 
		/*
		uart_puthl(to_ptr_pgd);
		uart_puthl(to_ptr_pgd);
		uart_puthl(to[i]);
		uart_puts(" ");
		*/
		for(int j = 0; j < 512; j++){
			if(ptr_pgd[j] == NULL)
				continue;
			ptr_pud = phy2vir(ptr_pgd[j] & 0xfffffffffffff000);
			if(to_ptr_pgd[j] == NULL)
				to_ptr_pgd[j] = page_table_init();
			to_ptr_pud = phy2vir(to_ptr_pgd[j] & 0xfffffffffffff000);
			/*
			uart_puthl(to_ptr_pgd[j]);
		uart_puthl(to_ptr_pud);
		uart_puts(" ");
		*/
			for(int k = 0; k < 512; k++){
				if(ptr_pud[k] == NULL)
					continue;
				ptr_pmd = phy2vir(ptr_pud[k] & 0xfffffffffffff000);
				if(to_ptr_pud[k] == NULL)
					to_ptr_pud[k] = page_table_init();
				to_ptr_pmd = phy2vir(to_ptr_pud[k] & 0xfffffffffffff000);
				/*
				uart_puts(" ");
		uart_puthl(to_ptr_pud[k]);
		uart_puthl(to_ptr_pmd);
		*/
		/*
		uart_puts(" ");
		*/
				for(int l = 0; l < 512; l++){
					if(ptr_pmd[l] == NULL)
						continue;
					/*
					if(to_ptr_pmd[l] == NULL)
						to_ptr_pmd[l] = page_table_init();
						*/
					to_ptr_pmd[l] = ptr_pmd[l];
					/*
					uart_puts("\n ptr_pmd");
					uart_puthl(ptr_pmd[l]);
					uart_puthl(to_ptr_pmd[l]);
					uart_puthl(to_ptr_pmd);
					*/
					/*
					char *f = (char*)phy2vir(ptr_pmd[l]);
					char *to = (char*)phy2vir(to_ptr_pmd[l]);
					uart_puts("\ninnnest");
					uart_puthl(f);
					uart_puthl(to);
					for(int z = 0; z < 0x1000; z++){
						*to++ = *f++;
					}
					*/
					//to_ptr_pmd[l] = ptr_pmd[l];
					//uart_puti(l);
				}
			}
		}
	}
	return 0;
}
		

