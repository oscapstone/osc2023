#include "uart.h"
#include "exception.h"

void sync_64_router(){
    unsigned long long spsr_el1;
	__asm__ __volatile__("mrs %0, SPSR_EL1\n\t" : "=r" (spsr_el1));

    unsigned long long elr_el1;
	__asm__ __volatile__("mrs %0, ELR_EL1\n\t" : "=r" (elr_el1));

    unsigned long long esr_el1;
	__asm__ __volatile__("mrs %0, ESR_EL1\n\t" : "=r" (esr_el1));

    uart_printf("exception sync_el0_64_router -> spsr_el1 : 0x%x, elr_el1 : 0x%x, esr_el1 : 0x%x\r\n",spsr_el1,elr_el1,esr_el1);

}


void invalid_exception_router(){
    uart_printf("invalid exception\n ");
}
