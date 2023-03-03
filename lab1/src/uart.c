#include "uart.h"

void uart_init() {
  *AUX_ENABLES |= 1;
  *AUX_MU_CNTL_REG = 0;
  *AUX_MU_IER_REG = 0;
  *AUX_MU_LCR_REG = 3;
  *AUX_MU_MCR_REG = 0;
  *AUX_MU_BAUD_REG = 270;
  *AUX_MU_IIR_REG = 0x06;

  // enable GPIO14, GPIO15
  register unsigned int r;
  r = *GPFSEL1;
  r &= ~((7 << 12) | (7 << 15));
  r |= (2 << 12) | (2 << 15);
  *GPFSEL1 = r;

  *GPPUD = 0;
  r = 150;
  while (r--) {
    asm volatile("nop");
  }
  *GPPUDCLK0 = (1 << 14) | (1 << 15);
  r = 150;
  while (r--) {
    asm volatile("nop");
  }
  *GPPUDCLK0 = 0;

  // enable TX, RX
  *AUX_MU_CNTL_REG = 3;
}

void uart_putchar(unsigned int c) {
  while (!(*AUX_MU_LSR_REG & 0x20)) {
    asm volatile("nop");
  }

  *AUX_MU_IO_REG = c;
}

char uart_getchar() {
  while (!(*AUX_MU_LSR_REG & 0x01)) {
    asm volatile("nop");
  }

  char res = (char)*AUX_MU_IO_REG;
  return res == '\r' ? '\n' : res;
}

void uart_puts(const char* s) {
  uart_printf(s);
  uart_printf("\n");
}

void uart_printf(const char* format, ...) {
  va_list va;
  va_start(va, format);

  // FIXME
  char buffer[4096];
  my_memset(buffer, 0, sizeof(buffer));
  my_vsprintf(buffer, format, va);

  va_end(va);

  char* p = buffer;
  while (*p) {
    if (*p == '\n') {
      uart_putchar('\r');
    }
    uart_putchar(*p);
    p++;
  }
}
