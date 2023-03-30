#include "mini_uart.h"

void uart_send(unsigned int c) {
  while ((*AUX_MU_LSR & 0x20) == 0);

  *AUX_MU_IO = c;
}

char uart_recv() {
  while ((*AUX_MU_LSR & 0x01) == 0);

  return (*AUX_MU_IO & 0xFF);
}

void uart_send_string(char* str) {
  for (int i = 0; str[i] != '\0'; i++) {
    uart_send((char)str[i]);
  }
}

void uart_init() {

  register unsigned int r;

  /* initialize UART */
  *AUX_ENABLE |= 1;       // enable UART1, AUX mini uart
  *AUX_MU_CNTL = 0;
  *AUX_MU_LCR = 3;       // 8 bits
  *AUX_MU_MCR = 0;
  *AUX_MU_IER = 0;
  *AUX_MU_IIR = 0xc6;    // disable interrupts
  *AUX_MU_BAUD = 270;    // 115200 baud

  /* map UART1 to GPIO pins */
  r = *GPFSEL1;
  r &= ~((7<<12)|(7<<15)); // gpio14, gpio15
  r |= (2<<12) | (2<<15);    // alt5
  *GPFSEL1 = r;
  *GPPUD = 0;            // enable pins 14 and 15
  r=150; while(r--) { asm volatile("nop"); }
  *GPPUDCLK0 = (1<<14)|(1<<15);
  r=150; while(r--) { asm volatile("nop"); }
  *GPPUDCLK0 = 0;        // flush GPIO setup
  *AUX_MU_CNTL = 3;      // enable Tx, Rx
}
