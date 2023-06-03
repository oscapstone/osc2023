#include "exception.h"
#include "uart.h"

// DAIF, Interrupt Mask Bits
void el1_interrupt_enable(){
    __asm__ __volatile__("msr daifclr, 0xf"); // umask all DAIF
}

void invalid_exception_router(unsigned long long x0){
    uart_send_string("invalid execption");
}

void el0_sync_router(){
    uart_send_string("in el0 sync");
    unsigned long long spsr_el1;
    __asm__ __volatile__("mrs %0, SPSR_EL1\n" : "=r" (spsr_el1)); // EL1 configuration, spsr_el1[9:6]=4b0 to enable interrupt
    unsigned long long elr_el1;
    __asm__ __volatile__("mrs %0, ELR_EL1\n" : "=r" (elr_el1));   // ELR_EL1: return address if return to EL1
    unsigned long long esr_el1;
    __asm__ __volatile__("mrs %0, ESR_EL1\n" : "=r" (esr_el1));   // ESR_EL1:symdrome information
    uart_send_string("[Exception][el0_sync] spsr_el1 :0x");
    uart_hex(spsr_el1);
    uart_send_string(" ,elr_el1 :0x");
    uart_hex(elr_el1);
    uart_send_string(" ,esr_el1 :0x");
    uart_hex(esr_el1);
    uart_send_string("\n");
}


void el1h_irq_router(){
    uart_send_string("hereeee");
    core_timer_handler();
}