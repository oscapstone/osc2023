#include "uart.h"
#include "gpio.h"

void uart_setup() {

  volatile unsigned int r = *GPFSEL1;
  r &= ~(7 << 12); // Clear gpio14
  r |= (2 << 12);  // gpio14 to alt5

  r &= ~(7 << 15); // Clear gpio15
  r |= (2 << 15);  // gpio15 to alt5

  *GPFSEL1 = r; // Write register
  *GPPUD = 0;	// disable pullup/down
  volatile register unsigned int t;
  t = 150;
  // Waiting for seting
  // The GPIO Pull-up/down Clock Registers control the actuation of internal pull-downs on
  // the respective GPIO pins. These registers must be used in conjunction with the GPPUD
  // register to effect GPIO Pull-up/down changes. The following sequence of events is
  // required:
  // 1. Write to GPPUD to set the required control signal (i.e. Pull-up or Pull-Down or neither
  // to remove the current Pull-up/down)
  // 2. Wait 150 cycles – this provides the required set-up time for the control signal
  // 3. Write to GPPUDCLK0/1 to clock the control signal into the GPIO pads you wish to
  // modify – NOTE only the pads which receive a clock will be modified, all others will
  // retain their previous state.
  // 4. Wait 150 cycles – this provides the required hold time for the control signal
  // 5. Write to GPPUD to remove the control signal
  // 6. Write to GPPUDCLK0/1 to remove the clock
  while(t--){
	  asm volatile("nop");
  }
  *GPPUDCLK0 = (1 << 14) | (1 << 15); //Setup clock for gp14, 15
  t = 150;

  while(t--){
	  asm volatile("nop");
  }
  *GPPUDCLK0 = 0;


  *AUX_ENABLE |= 1;   // 1 -> AUX mini Uart
  *AUX_MU_CNTL = 0;   // Disable Tx/Rx
  *AUX_MU_LCR = 3;    // Set data to 8-bit mode
  *AUX_MU_MCR = 0;    // Ignore
  *AUX_MU_IER = 0;    // Init
  *AUX_MU_IIR = 0xc6;  // No timeout + clear FIFO
  *AUX_MU_BAUD = 270; // 115200

  
  *AUX_MU_CNTL = 3;   // Enable tr and re.
  return;
}

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

// Load the kernel from uart to 0x80000
void read_kernel(){
	// Initial uart
	uart_setup();
	uart_puts("Wait for kernel!\n");
	volatile char* ke_lo = (volatile char*)0x80000;
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

	if(size < 64 || size > 1024 * 1024){
		uart_puts("TOO large, Try again.\n");
		goto redo;
	}
	uart_puts("Good, start kernel.\n");

	while(size--){
		*ke_lo++  = uart_get();
		uart_puti(size);
	}
	uart_puts("Kernel transmition Done.\n");
	 asm volatile(
	 		
	 	"mov x0, x10;"
	 	"mov x1, x11;"
	 	"mov x2, x12;"
	 	"mov x3, x13;"
		"mov x4, #0x80000;"
		"br x4;"
	 	"mov x30, 0x80000;"
	 	"ret"
	 );
	return ;
}



