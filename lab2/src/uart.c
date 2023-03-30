#include "aux.h"
#include "gpio.h"
#include "uart.h"
#include "my_str.h"

void uart_init()
{
	register unsigned int r;
	*AUX_ENABLE |= 1; //enable mini uart
	*AUX_MU_CNTL = 0; // disable TX/RX during configuration
	*AUX_MU_IER = 0;  // disable interrupt
	*AUX_MU_LCR = 3; // 8 bits
	*AUX_MU_MCR = 0;  //do not need auto flow control
	*AUX_MU_BAUD = 270; // 115200 baud
	*AUX_MU_IIR = 6; // no FIFO
	/* map UART1 to GPIO pins */
	r=*GPFSEL1;
	r&=~((7<<12)|(7<<15)); // reset gpio14, gpio15
	r|=(2<<12)|(2<<15);    // set alt5 for gpio14, 15
	*GPFSEL1 = r;
	*GPPUD = 0;            // enable pins 14 and 15
	r=150; while(r--) { asm volatile("nop"); } // delay 150 clocks
	*GPPUDCLK0 = (1<<14)|(1<<15);  // no pull-up no pull-down
	r=150; while(r--) { asm volatile("nop"); }
	*GPPUDCLK0 = 0;        // flush GPIO setup
	*AUX_MU_CNTL = 3;

}
// send character
void uart_write(unsigned int c){
	/* wait until we can send */
	do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x20));
	/* write the character to the buffer */
	*AUX_MU_IO=c;
}
// read character
char uart_read(){
	char r;
	/* wait until something is in the buffer */
	do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x01));
	/* read it and return */
	r=(char)(*AUX_MU_IO);
	/* convert carriage return to newline */
	return r=='\r'?'\n':r;
}
char uart_read_raw(){
	char r;
	/* wait until something is in the buffer */
	do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x01));
	/* read it and return */
	r = (char)(*AUX_MU_IO);
	return r;
}

// display string

int uart_printf(char* fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    char buf[VSPRINT_MAX_BUF_SIZE];

    char *str = (char*)buf;
    int count = vsprintf(str,fmt,args);

    while(*str) {
        if(*str=='\n')
            uart_write('\r');
        uart_write(*str++);
    }
    __builtin_va_end(args);
    return count;
}

void uart_printf_n(char *s, int len){
    for(int i=0; i<len; i++) {
        /* convert newline to carrige return + newline */
      if (*s == '\n') uart_write('\r');
      uart_write(*s++);	    
    }
}
void uart_flush(){
	do{*AUX_MU_IO;}while((*AUX_MU_LSR&0x01));
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
        uart_write(n);
    }
}