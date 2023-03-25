#include "uart.h"
#include "gpio.h"
#include "interrupt.h"
#include "str.h"

#define uart_buf_len 256
static char rx_buf[uart_buf_len];
static char tx_buf[uart_buf_len];
static int rx_point = 0;
static int tx_point = 0;

/*=======================================================================+
 | Enable or Disable mini uart interrupt                                 |
 +======================================================================*/

/*************************************************************************
 * Disable Recieve interrupt without reset the rx_point
 ************************************************************************/
int disable_uart_receive_int(void) {
	*AUX_MU_IER &= 0x02;
	return 0;
}

/**************************************************************************
 * Enable Recieve interrupt
 *************************************************************************/
int enable_uart_receive_int(void) {
  //rx_point = 0;        // Initialize the pivot
  *AUX_MU_IER |= 0x01; // Enable Rx interrupt
  return 0;
}

/*************************************************************************
 * Disable Transmit interrupt without reset the rx_point
 ************************************************************************/
int disable_uart_transmit_int(void) {
	*AUX_MU_IER &= 0x01;	// Disalbe bit 2.
	return 0;
}

/**************************************************************************
 * Enable Transmit interrupt
 *************************************************************************/
int enable_uart_transmit_int(void) {
  *AUX_MU_IER |= 0x02; 	// Enable Tx interrupt
  return 0;
}
/*=======================================================================*/

int reset_rx(void){
	rx_point = 0;
	return 0;
}

/**************************************************************************
 * The interrupt handler of mini Uart Receive.
 *
 * Read all input into the rx_buf.
 *************************************************************************/
int uart_receive_handler() {
  // uart_puts("receive\n");
  if (rx_point >= uart_buf_len - 1)
    rx_point %= uart_buf_len;
  rx_buf[rx_point++] = (char)(*AUX_MU_IO);
  return 0;
}

/**************************************************************************
 * Interrupt handler of mini Uart Transmit
 *
 * Put all contents into uart and disable the TX interupt.
 *************************************************************************/
int uart_transmit_handler() {
  // uart_puts("transmit\n");
  if (tx_point < uart_buf_len && tx_buf[tx_point] != 0){
    *AUX_MU_IO = tx_buf[tx_point++]; // Write to buffer
    enable_uart_transmit_int();	// Still have chars in the buffer.
  }
  else
    *AUX_MU_IER &= 0x01; // Transmition done disable interrupt.
  return 0;
}

/**************************************************************************
 * Interrupt version of sending string through UART
 *************************************************************************/
int uart_a_puts(const char *str, int len) {
  uart_puts("async write:\n");
  if (len <= 0)
    return 1;
  tx_point = 0;
  for (int i = 0; i < len; i++) {
    tx_buf[i] = str[i];
  }
  tx_buf[len] = 0;
  *AUX_MU_IER |= 0x02; // Enable Tx interrupt.
  return 0;
}

/**************************************************************************
 * Interrupt version of geting string
 * Should close the Rx interrupt after this function called.
 *************************************************************************/
int uart_a_gets(char *str, int len) {
  *AUX_MU_IER &= 0x02; // Disable Rx interrupt.
  if (len <= 0)
    return 1;
  for (int i = 0; i < rx_point && i < len; i++) {
    str[i] = rx_buf[i];
  }
  rx_point = 0;
  return 0;
}

void uart_setup() {

  volatile unsigned int r = *GPFSEL1;
  r &= ~(7 << 12); // Clear gpio14
  r |= (2 << 12);  // gpio14 to alt5

  r &= ~(7 << 15); // Clear gpio15
  r |= (2 << 15);  // gpio15 to alt5

  *GPFSEL1 = r; // Write register
  *GPPUD = 0;   // disable pullup/down
  volatile register unsigned int t;
  t = 150;
  // Waiting for seting
  // The GPIO Pull-up/down Clock Registers control the actuation of internal
  // pull-downs on the respective GPIO pins. These registers must be used in
  // conjunction with the GPPUD register to effect GPIO Pull-up/down changes.
  // The following sequence of events is required:
  // 1. Write to GPPUD to set the required control signal (i.e. Pull-up or
  // Pull-Down or neither to remove the current Pull-up/down)
  // 2. Wait 150 cycles – this provides the required set-up time for the control
  // signal
  // 3. Write to GPPUDCLK0/1 to clock the control signal into the GPIO pads you
  // wish to modify – NOTE only the pads which receive a clock will be modified,
  // all others will retain their previous state.
  // 4. Wait 150 cycles – this provides the required hold time for the control
  // signal
  // 5. Write to GPPUD to remove the control signal
  // 6. Write to GPPUDCLK0/1 to remove the clock
  while (t--) {
    asm volatile("nop");
  }
  *GPPUDCLK0 = (1 << 14) | (1 << 15); // Setup clock for gp14, 15
  t = 150;

  while (t--) {
    asm volatile("nop");
  }
  *GPPUDCLK0 = 0;

  *AUX_ENABLE |= 1;   // 1 -> AUX mini Uart
  *AUX_MU_CNTL = 0;   // Disable Tx/Rx
  *AUX_MU_LCR = 3;    // Set data to 8-bit mode
  *AUX_MU_MCR = 0;    // Ignore
  *AUX_MU_IER = 0x0;  // Enable both T/R interrupts(bit1/0).
  *AUX_MU_IIR = 0xc6; // No timeout + clear FIFO
  *AUX_MU_BAUD = 270; // 115200

  *AUX_MU_CNTL = 3; // Enable tr and re.
  return;
}

/**************************************************************************
 * Send char to uart. RAW version.
 *************************************************************************/
void uart_send(unsigned int c) {
  do {
    asm volatile("nop");
  } while (!(*AUX_MU_LSR & 0x20)); // Check If buffer is empty
  *AUX_MU_IO = c;                  // Write to buffer
}

char uart_get() {
  char c;
  do {
    asm volatile("nop");
  } while (!(*AUX_MU_LSR & 0x01)); // Check If data to read
  c = (char)(*AUX_MU_IO);
  return c;
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

void uart_putsn(char *s, int n) {
  while (n-- > 0) {
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

void l2s_r(char **p, long int i) {

  char d = (i % 10) + '0';
  if (i >= 10)
    l2s_r(p, i / 10);
  *((*p)++) = d;
}

void uart_puti(unsigned int i) {
  static char buf[24];
  char *p = buf;
  if (i < 0) {
    *p++ = '-';
    i = 0 - i;
  }
  l2s_r(&p, i);
  *p = '\0';
  uart_puts(buf);
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

// For 64 bit hex output
void uart_puthl(uint64_t h) {
  unsigned int n;
  int c;
  for (c = 60; c >= 0; c -= 4) {
    n = (h >> c) & 0xf;
    n += n > 9 ? 0x37 : '0';
    uart_send(n);
  }
  return;
}

// Load the kernel from uart to 0x80000
void read_kernel() {
  // Initial uart
  uart_setup();
  uart_puts("Wait for kernel!\n");
  volatile char *ke_lo = (volatile char *)0x80000;
  int size = 0;
  // Get the kernel size
redo:
  size = 0;
  size = uart_getc() - '0';
  uart_puts("current size:\n");
  uart_puti(size);
  size *= 10;
  size += uart_getc() - '0';
  uart_puts("current size:\n");
  uart_puti(size);
  size *= 10;
  size += uart_getc() - '0';
  uart_puts("current size:\n");
  uart_puti(size);
  size *= 10;
  size += uart_getc() - '0';
  uart_puts("current size:\n");
  uart_puti(size);
  size *= 10;
  size += uart_getc() - '0';
  uart_puts("current size:\n");
  uart_puti(size);

  if (size < 64 || size > 1024 * 1024) {
    uart_puts("TOO large, Try again.\n");
    goto redo;
  }
  uart_puts("Good, start kernel.\n");

  while (size--) {
    *ke_lo++ = uart_get();
    uart_putc('o');
  }
  uart_puts("Kernel transmition Done.\n");
  asm volatile(

      "mov x0, x25;"
      "mov x1, x11;"
      "mov x2, x12;"
      "mov x3, x13;"
      "mov x30, #0x80000;"
      "ret");
  return;
}
