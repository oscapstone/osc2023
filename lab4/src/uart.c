#include "gpio.h"

/* Auxilary mini UART registers */
#define AUX_ENABLE      ((volatile unsigned int*)(MMIO_BASE+0x00215004))
#define AUX_MU_IO       ((volatile unsigned int*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER      ((volatile unsigned int*)(MMIO_BASE+0x00215044))
#define AUX_MU_IIR      ((volatile unsigned int*)(MMIO_BASE+0x00215048))
#define AUX_MU_LCR      ((volatile unsigned int*)(MMIO_BASE+0x0021504C))
#define AUX_MU_MCR      ((volatile unsigned int*)(MMIO_BASE+0x00215050))
#define AUX_MU_LSR      ((volatile unsigned int*)(MMIO_BASE+0x00215054))
#define AUX_MU_MSR      ((volatile unsigned int*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH  ((volatile unsigned int*)(MMIO_BASE+0x0021505C))
#define AUX_MU_CNTL     ((volatile unsigned int*)(MMIO_BASE+0x00215060))
#define AUX_MU_STAT     ((volatile unsigned int*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD     ((volatile unsigned int*)(MMIO_BASE+0x00215068))

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
    *AUX_ENABLE |=1;       // enable UART1, AUX mini uart
    *AUX_MU_CNTL = 0;
    *AUX_MU_LCR = 3;       // 8 bits
    *AUX_MU_MCR = 0;
    *AUX_MU_IER = 0;
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


