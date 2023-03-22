#include "uart1.h"
#include "exception.h"
#include "timer.h"

void el0_sync_router(){
    unsigned long long spsr_el1;
    __asm__ __volatile__("mrs %0, SPSR_EL1\n\t" : "=r" (spsr_el1) :  : "memory");

    unsigned long long elr_el1;
    __asm__ __volatile__("mrs %0, ELR_EL1\n\t" : "=r" (elr_el1) :  : "memory");

    unsigned long long esr_el1;
    __asm__ __volatile__("mrs %0, ESR_EL1\n\t" : "=r" (esr_el1) :  : "memory");

    uart_puts("## Exception - el0_sync ## spsr_el1 : 0x%x, elr_el1 : 0x%x, esr_el1 : 0x%x\n", spsr_el1, elr_el1, esr_el1);
}

void el0_irq_64_router(){
    //uart_puts("source : %x\n", *CORE0_INTERRUPT_SOURCE);

    if(*CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_CNTPNSIRQ)  //from CNTPNS (core_timer)
    {
        core_timer_handler();
    }
}

void invalid_exception_router(unsigned long long x0){
    uart_puts("invalid exception\n : %d",x0);
}
