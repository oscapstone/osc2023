#include "utils.h"
#include "uart.h"
#include "exception.h"
//later lab: https://oscapstone.github.io/labs/lab3.html#exception-handling
void default_handler()
{
    disable_local_all_interrupt();

    unsigned long spsr = read_sysreg(spsr_el1);
    unsigned long elr = read_sysreg(elr_el1);
    unsigned long esr = read_sysreg(esr_el1);
    uart_write_string("spsr_el1: ");
    uart_write_no_hex(spsr);
    uart_write_string("\n");

    uart_write_string("elr_el1: ");
    uart_write_no_hex(elr);
    uart_write_string("\n");

    uart_write_string("esr_el1: ");
    uart_write_no_hex(esr);
    uart_write_string("\n");
    
    enable_local_all_interrupt();
}