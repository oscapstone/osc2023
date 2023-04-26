#include "gpio.h"
#include "uart.h"
#include "interrupt.h"

#define max_len 256

static char read_buf[max_len];
static char write_buf[max_len];
static int rx = 0;
static int tx = 0;
static int count;

int uart_receive_handler(){
	uart_puts("Receive\n");
    uart_puts("Characters: ");
    
	if(rx >= max_len - 1)
		rx %= max_len;
  	read_buf[rx] = (char)(*AUX_MU_IO);

    uart_putc(read_buf[rx++]);
    uart_puts("\n");

	return 0;
}

int uart_transmit_handler(){
	uart_puts("Transmit\n");
    uart_puts("Characters: ");
    uart_putc(write_buf[count]);
    uart_puts("\n");

	if(tx >= 0) {
		*AUX_MU_IO = write_buf[count++];     // Write to buffer
        tx--;
    }
	else {
  		*AUX_MU_IER &= 0x01;    // Transmition done disable transmit interrupt. 
    }

	return 0;
}

int uart_a_puts(const char* str, int len){
	if(len <= 0)
		return 1;
	tx = len;
    count = 0;
	for(int i = 0; i < len; i++){
		write_buf[i] = str[i];
	}
	*AUX_MU_IER |= 0x03;		// Enable Tx interrupt.
	return 0;
}

int uart_a_gets(char* str, int len) {
	if(len <= 0)
		return 1;
	for(int i = 0; i < rx && i < len; i++){
		str[i] = read_buf[i];
	}
	rx = 0;
	return 0;
}


/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart_init()
{
    register unsigned int r;
    /* map UART1 to GPIO pins */
    r=*GPFSEL1;
    r&=~((7<<12)|(7<<15)); // gpio14, gpio15
    r|=(2<<12)|(2<<15);    // alt5
    *GPFSEL1 = r;

    *GPPUD = 0;            // enable pins 14 and 15
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = (1<<14)|(1<<15);
    r=150; while(r--) { asm volatile("nop"); }

    *GPPUDCLK0 = 0;        // flush GPIO setup
    /* initialize UART */
    *AUX_ENABLE |= 1;       // enable UART1, AUX mini uart
    *AUX_MU_CNTL = 0;
    *AUX_MU_LCR = 3;       // 8 bits
    *AUX_MU_MCR = 0;
    *AUX_MU_IER = 0x0;
    *AUX_MU_IIR = 0xc6;    // disable interrupts
    *AUX_MU_BAUD = 270;    // 115200 baud
    *AUX_MU_CNTL = 3;      // enable Tx, Rx
}

void uart_send(unsigned int c) {
    while(1)
    {
        if(*AUX_MU_LSR&0x20)
        {
                break;
        }
    }
    *AUX_MU_IO=c;
}

char uart_getc() {
    char r;
    while(1)
    {
	if(*AUX_MU_LSR&0x01)
	{
		break;
	}
    } 
    r=(char)(*AUX_MU_IO);
    return r=='\r'?'\n':r;
}

void uart_putc(char c) {
  if (c == '\n') {
    uart_send('\n');
    uart_send('\r');
  }
  uart_send(c);
}

void uart_puts(char *s) {
    while(*s) {
        if(*s=='\n')
            uart_send('\r');
        uart_send(*s++);
    }
}

void uart_putsn(char *s, int n) {                               
    while (n-- > 0) {                                             
        if (*s == '\n') {
            uart_send('\r');
        }                                                       
        uart_send(*s++);                                
    }
}

// Display a binary value in hexadecimal
void uart_hex(unsigned int d) {
    unsigned int n;
    int c;
    for(c=28;c>=0;c-=4) {
        // get highest tetrad
        n=(d>>c)&0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n+=n>9?0x37:0x30;
        uart_send(n);
    }
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