#include <exception.h>

#include <string.h>
#include <stdint.h>

#include <uart.h>

void exception_init(){
    asm("msr vbar_el1, %0"::"r"((void*)&exception_vector_table));
}

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
}