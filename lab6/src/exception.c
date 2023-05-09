#include "exception.h"
#include "uart.h"

static uint64_t read_spsr_el1(void) {
  uint64_t spsr;
  asm volatile("mrs	%0, spsr_el1;" : "=r"(spsr));
  return spsr;
}

static uint64_t read_far_el1(void) {
  uint64_t far;
  asm volatile("mrs	%0, far_el1;" : "=r"(far));
  return far;
}

static uint64_t read_ttbr1_el1(void) {
  uint64_t far;
  asm volatile("mrs	%0, ttbr1_el1;" : "=r"(far));
  return far;
}

static uint64_t read_ttbr0_el1(void) {
  uint64_t far;
  asm volatile("mrs	%0, ttbr0_el1;" : "=r"(far));
  return far;
}

static uint64_t read_elr_el1(void) {
  uint64_t elr_el1;
  asm volatile("mrs	%0, elr_el1;" : "=r"(elr_el1));
  return elr_el1;
}

static uint64_t read_esr_el1(void) {
  uint64_t esr_el1;
  asm volatile("mrs	%0, esr_el1;" : "=r"(esr_el1));
  return esr_el1;
}

int print_elr_el1(void) {
  uart_puts("elr_el1:\t");
  uart_puthl(read_elr_el1());
  uart_puts("\n");
  return 0;
}
int print_far_el1(void) {
  uart_puts("far_el1:\t");
  uart_puthl(read_far_el1());
  uart_puts("\n");
  return 0;
}
int print_ttbr0_el1(void) {
  uart_puts("ttbr0_el1:\t");
  uart_puthl(read_ttbr0_el1());
  uart_puts("\n");
  return 0;
}
int print_ttbr1_el1(void) {
  uart_puts("ttbr1_el1:\t");
  uart_puthl(read_ttbr1_el1());
  uart_puts("\n");
  return 0;
}

int print_esr_el1(void) {
  uart_puts("esr_el1:\t");
  uart_puthl(read_esr_el1());
  uart_puts("\n");
  return 0;
}

int print_spsr_el1(void) {
  uart_puts("spsr_el1:\t");
  uart_puthl(read_spsr_el1());
  uart_puts("\n");
  return 0;
}

int exception_entry() {

  uart_puts("Other exceptions\n");
  print_spsr_el1();
  print_elr_el1();
  print_esr_el1();
  print_far_el1();
  print_ttbr0_el1();
  print_ttbr1_el1();
  

  return 0;
}
