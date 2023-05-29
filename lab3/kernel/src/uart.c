#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"

//initial uart
void uart_init()
{
    register unsigned int r;

    /* initialize UART */
    *AUX_ENABLES |=1;       // enable UART1, AUX mini uart
    *AUX_MU_CNTL = 0;
    *AUX_MU_LCR = 3;       // 8 bits
    *AUX_MU_MCR = 0;
    *AUX_MU_IER = 0;
    *AUX_MU_IIR = 6;    // disable interrupts
    *AUX_MU_BAUD = 270;    // 115200 baud
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
    *AUX_MU_CNTL = 3;      // enable Tx, Rx
}


//Send a character(in int) to console
void uart_send(unsigned int c) {
    /* wait until we can send */
    // LSR: bit 5 => transmitter empty  0x20(32)
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x20));
    /* write the character to the buffer; display on console */
    *AUX_MU_IO=c;
}


 //Receive a character from console
char uart_recv() {
    char r;
    /* wait until something is in the buffer */
    //nop: no option in assembly; LSR:line status bit 0 => data ready 
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x01));

    /* read it and return */
    r=(char)(*AUX_MU_IO);
    /* convert carriage return to newline */

    return r=='\r'?'\n':r;
}

/**
 * Display a string to console
 */
void uart_send_string(char *s) {
    while(*s) {
        /* convert newline to carriage return + newline */
        if(*s=='\n')
            uart_send('\r');
        uart_send(*s++);
    }
}

// to hex
void uart_hex(unsigned int d) {
    unsigned int n;
    int c;
    for(c=28;c>=0;c-=4) {
        // get highest tetrad
        n=(d>>c)&0xF; // 1111 
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n+=n>9?0x37:0x30; //>9 +55  <9 +48 to ascii 0-9
        uart_send(n);
    }
}


//flush
void uart_flush(){
    while ( *AUX_MU_LSR & 0x01 ){ //if data exist 
        char r = *AUX_MU_IO; //clean
    }
}

//uart read raw
char uart_read_raw() {
    char r;
    /* wait until something is in the buffer */
    do{
      asm volatile("nop");
    } while ( !( *AUX_MU_LSR&0x01 ) );
    /* read it and return */
    r = (char)(*AUX_MU_IO);
    return r;
}

//send cat file:
void uart_send_cat_file(char *s, int len){
    for(int i=0; i<len; i++) {
        /* convert newline to carrige return + newline */
      if (*s == '\n') uart_send('\r');
      uart_send(*s++);     
    }
}

