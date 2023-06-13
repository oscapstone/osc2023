#include "exception.h"
#include "timer.h"
#include "uart.h"

// print the content of spsr_el1, elr_el1, and esr_el1 in the exception handler. 
void exception_entry(){
  unsigned long spsr_el1, elr_el1, esr_el1;
  asm volatile("mrs %0, spsr_el1"
               : "=r" (spsr_el1));
  asm volatile("mrs %0, elr_el1"
               : "=r" (elr_el1));
  asm volatile("mrs %0, esr_el1"
               : "=r" (esr_el1));
  uart_puts("SPSR_EL1: ");
  uart_hex(spsr_el1);
  uart_puts("\n\rELR_EL1: ");
  uart_hex(elr_el1);
  uart_puts("\n\rESR_EL1: ");
  uart_hex(esr_el1);
  uart_puts("\n\r");
}


void boot_time_2_sec(){
  time_elapsed();
  two_sec_interrupt();
}
void irq_exception_router(){
  unsigned long irq_pending;
  unsigned long source_core0_irq;
  irq_pending = *IRQ_PENDING_1;
  source_core0_irq = *SRC_CORE0_INTERRUPT;
  // core timer interrupt
  if(source_core0_irq & SRC_CPU_TIMER){
    // need the timer interrupt handler for el1
    core_timer_handle();
  }
  // uart interrupt
  else if ((irq_pending & GPU_IRQ_ENTRY) && (source_core0_irq & SRC_GPU_INTERRUPT)){
    async_uart_handle();
  }
}


