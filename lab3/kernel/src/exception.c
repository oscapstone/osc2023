#include "exception.h"
#include "uart.h"

void invalid_exception_router(unsigned long long x0){

}

void el0_sync_router(){
    unsigned long long spsr_el1;
    __asm__ __volatile__("mrs %0, SPSR_EL1\n\t" : "=r" (spsr_el1)); // EL1 configuration, spsr_el1[9:6]=4b0 to enable interrupt
    unsigned long long elr_el1;
    __asm__ __volatile__("mrs %0, ELR_EL1\n\t" : "=r" (elr_el1));   // ELR_EL1 holds the address if return to EL1
    unsigned long long esr_el1;
    __asm__ __volatile__("mrs %0, ESR_EL1\n\t" : "=r" (esr_el1));   // ESR_EL1 holds symdrome information of exception, to know why the exception happens.
    uart_send_string("[Exception][el0_sync] spsr_el1 :");
    uart_hex(spsr_el1);
    uart_send_string(" ,elr_el1 :");
    uart_hex(elr_el1);
    uart_send_string(" ,esr_el1 :");
    uart_hex(esr_el1);
    uart_send_string("\n");
}
