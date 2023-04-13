#include "exception.h"
#include "mini_uart.h"
#include "mailbox.h"
#include "utils.h"
#include "timer.h"
#include "peripherals/exception.h"
#include "peripherals/mini_uart.h"


/*msr(write) mrs(read) are used in particular register*/

void enable_interrupt() {asm volatile("msr DAIFClr, 0xf");}     //trun on FIQ IRQ Async Dbg
void disable_interrupt() {asm volatile("msr DAIFSet, 0xf");}    //trun off FIQ IRQ Async Dbg

void exception_entry() {
    disable_interrupt();
    unsigned long long spsr_el1;
	asm volatile("mrs %0, SPSR_EL1\n\t" : "=r" (spsr_el1) :  : "memory");

    unsigned long long elr_el1;
	asm volatile("mrs %0, ELR_EL1\n\t" : "=r" (elr_el1) :  : "memory");

    unsigned long long esr_el1;
	asm volatile("mrs %0, ESR_EL1\n\t" : "=r" (esr_el1) :  : "memory");
    
    uart_send_string("spsr_el1: ");
    printhex(spsr_el1);
    uart_send_string("\n");
    
    uart_send_string("elr_el1: ");
    printhex(elr_el1);
    uart_send_string("\n");
    
    uart_send_string("esr_el1: ");
    printhex(esr_el1);
    uart_send_string("\n");
    while(1);
}

/*---Exception Handling---*/
//sync_64_EL0_handler
void sync_64_router() {
    disable_interrupt();
    unsigned long long spsr_el1;
	asm volatile("mrs %0, SPSR_EL1\n\t" : "=r" (spsr_el1) :  : "memory");

    unsigned long long elr_el1;
	asm volatile("mrs %0, ELR_EL1\n\t" : "=r" (elr_el1) :  : "memory");

    unsigned long long esr_el1;
	asm volatile("mrs %0, ESR_EL1\n\t" : "=r" (esr_el1) :  : "memory");
    
    uart_send_string("spsr_el1: ");     // EL1 configuration, spsr_el1[9:6]=4b0 to enable interrupt
    printhex(spsr_el1);
    uart_send_string("\n");
    
    uart_send_string("elr_el1: ");      // ELR_EL1 holds the address if return to EL1
    printhex(elr_el1);
    uart_send_string("\n");
    
    uart_send_string("esr_el1: ");      // ESR_EL1 holds symdrome information of exception, to know why the exception happens.
    printhex(esr_el1);
    uart_send_string("\n");
    enable_interrupt();
}

void irq_exc_router() {
    disable_interrupt();
    if (get32(IRQ_PENDING_1)&IRQ_PENDING_1_AUX_INT && get32(CORE0_INTERRUPT_SRC)&INTERRUPT_SOURCE_GPU) {  // from aux && from GPU0 -> uart exception

        async_uart_handler();

    } else if (get32(CORE0_INTERRUPT_SRC)&INTERRUPT_SOURCE_CNTPNSIRQ) {     //from CNTPNS (core_timer) // A1 - setTimeout run in el1

        pop_timer();

    }
    enable_interrupt();
}


/*---Core Timer Interrupt---*/
//irq_64_EL0_handler
void two_btime_handler() {

    time_elapsed();

    asm volatile(
        "mrs x0, cntfrq_el0\n\t"
        "mov x1, 2\n\t"     //per 2 second
        "mul x0, x0, x1\n\t"
        "msr cntp_tval_el0, x0\n\t"
    );

}