#include <interrupt.h>

#include <stdint.h>
#include <string.h>

#include <uart.h>
#include <utils.h>
#include <coretimer.h>

uint32_t interrupt_depth;

void interrupt_enable(){
    if(interrupt_depth==0) asm("msr DAIFClr, 0xf");
    interrupt_depth++;
}

void interrupt_disable(){
    interrupt_depth--;
    if(interrupt_depth == 0) asm("msr DAIFSet, 0xf");
}
#include <command.h>
void exception_handler(){
    uint64_t spsr_el1;
    uint64_t elr_el1;
    uint64_t esr_el1;
    uint64_t x0;
    asm("mov %0, x0":"=r"(x0));
    asm("mrs %0, spsr_el1":"=r"(spsr_el1));
    asm("mrs %0, elr_el1":"=r"(elr_el1));
    asm("mrs %0, esr_el1":"=r"(esr_el1));
    uart_print("x0: ");
    uart_print_hex(x0, 64);
    newline();
    uart_print("spsr_el1: ");
    uart_print_hex(spsr_el1, 64);
    newline();
    uart_print("elr_el1: ");
    uart_print_hex(elr_el1, 64);
    newline();
    uart_print("esr_el1: ");
    uart_print_hex(esr_el1, 64);
    newline();
    reboot();
}

void interrupt_irq_handler(){
    uint32_t irq_pend_base_reg = memory_read(ARMINT_IRQ_PEND_BASE_REG);
    uint32_t irq_pend1_reg = memory_read(ARMINT_IRQ_PEND1_REG);
    uint32_t irq_pend2_reg = memory_read(ARMINT_IRQ_PEND2_REG);
    uint32_t core0_int_src = memory_read(CORE0_INTERRUPT_SOURCE);
    uint32_t aux_mu_iir_reg = memory_read(AUX_MU_IIR_REG);

    if(core0_int_src & 0b10){
        coretimer_el0_handler();
    }
    if(aux_mu_iir_reg&0b100){
        uart_interrupt_handler();
    }
}

void not_implemented_interrupt(){
    uart_print("Not implemented!!!");
    newline();
}
