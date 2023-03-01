#include "uart.h"
#include "gpio.h"

void uart_setup() {

  unsigned int r = *GPFSEL1;
  r &= ~(7 << 12); // Clear gpio14
  r |= (2 << 12);  // gpio14 to alt5

  r &= ~(7 << 15); // Clear gpio15
  r |= (2 << 15);  // gpio15 to alt5

  *GPFSEL1 = r; // Write register

  *AUX_ENABLE |= 1;   // 1 -> AUX mini Uart
  *AUX_MU_CNTL = 0;   // Disable Tx/Rx
  *AUX_MU_LCR = 3;    // Set data to 8-bit mode
  *AUX_MU_MCR = 0;    // Ignore
  *AUX_MU_IER = 0;    // Init
  *AUX_MU_IIR = 0x6;  // No timeout + clear FIFO
  *AUX_MU_BAUD = 270; //
  *AUX_MU_CNTL = 3;   // Enable tr and re.
}

void uart_send(unsigned int c) {
  do {
    asm volatile("nop");
  } while (!(*AUX_MU_LSR & 0x20)); // Check If buffer is empty
  *AUX_MU_IO = c;                  // Write to buffer
}

char uart_getc() {
  char c;
  do {
    asm volatile("nop");
  } while (!(*AUX_MU_LSR & 0x01)); // Check If data to read
  c = (char)(*AUX_MU_IO);
  return c == '\r' ? '\n' : c;
}

void uart_puts(char *s) {
  while (*s) {
    if (*s == '\n') {
      uart_send('\r');
    }
    uart_send(*s++);
  }
  return;
}
void uart_putc(char c) {
  if (c == '\n') {
    uart_send('\n');
    uart_send('\r');
    return;
  }
  uart_send(c);
}

void uart_puti(unsigned int i) {
  if (i == 0) {
    uart_send('\0');
    return;
  }

  while (i > 0) {
    uart_send(i % 10 + '0');
    i = i / 10;
  }
  return;
}

void uart_puth(unsigned int h) {
  unsigned int n;
  int c;
  for (c = 28; c >= 0; c -= 4) {
    n = (h >> c) & 0xf;
    n += n > 9 ? 0x37 : '0';
    uart_send(n);
  }
  return;
}
