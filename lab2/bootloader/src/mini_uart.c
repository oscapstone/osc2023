#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"
#include "peripherals/mailbox.h"

/* mailbox message buffer */
volatile unsigned int  __attribute__((aligned(16))) mbox[36];

/**
 * initialize UART
 */
void uart_init ( void ) {
    register unsigned int r;
    r=*GPFSEL1;
    r&=~((7<<12)|(7<<15)); // gpio14, gpio15
    r|=(2<<12)|(2<<15);    // alt5
    *GPFSEL1 = r;
    *GPPUD = 0;            // enable pins 14 and 15

    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = (1<<14)|(1<<15);
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = 0;        // flush GPIO setup
    *AUX_MU_CNTL_REG = 3;      // enable Tx, Rx

    *AUX_ENABLES |=1;       // enable UART1, AUX mini uart
    *AUX_MU_CNTL_REG = 0;
    *AUX_MU_LCR_REG = 3;       // 8 bits
    *AUX_MU_MCR_REG = 0;
    *AUX_MU_IER_REG = 0;
    *AUX_MU_IIR_REG = 6;
    *AUX_MU_BAUD_REG = 270;    // 115200 baud
    *AUX_MU_CNTL_REG = 3;
    /* map UART1 to GPIO pins */
}

/**
 * Display a character
 */
void uart_send(char c) {
    /* wait until we can send */
    do{asm volatile("nop");}while(!(*AUX_MU_LSR_REG&0x20));
    /* write the character to the buffer */
    *AUX_MU_IO_REG=c;
}

/**
 * Receive a character
 */
char uart_getc() {
    char r;
    /* wait until something is in the buffer */
    do{asm volatile("nop");}while(!(*AUX_MU_LSR_REG&0x01));
    /* read it and return */
    r=(char)(*AUX_MU_IO_REG);
    /* convert carriage return to newline */
    return r=='\r'?'\n':r;
}

char uart_getc_pure() {
    char r;
    /* wait until something is in the buffer */
    do{asm volatile("nop");}while(!(*AUX_MU_LSR_REG&0x01));
    /* read it and return */
    r=(char)(*AUX_MU_IO_REG);
    /* convert carriage return to newline */
    return r;
}

/**
 * Display a string. The '\r\n' issue is handled in this function
 */
void uart_puts(char *s) {
	for (int i = 0; s[i] != '\0'; i++) {
        if(s[i]=='\n')
            uart_send('\r');
		uart_send((char)s[i]);
	}
}

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

void uart_hexdump(unsigned int d) {
    unsigned int n;
    int c;
    for(c=4;c>=0;c-=4) {
        // get highest tetrad
        n=(d>>c)&0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n+=n>9?0x37:0x30;
        uart_send(n);
    }
    uart_send(' ');
}

int atoi(char * ch){
    int ret = 0;
    for(int i = 0; i<5; i++){
        ret*=10;
        ret+=(ch[i]-'0');
    }
    // while(*ch != '\0'){
    //     ret*=10;
    //     ret += (*ch-'0');
    //     ch++;
    // }
    // return ret;
}

int exp(int i, int j){
    int ret = 1;
    for (; j>0; j--){
        ret*=i;
    }
    return ret;
}

void uart_int(int i){
    int e = 0;
    int temp = i;
    while ((i /= 10) >0){
        e++;
    }
    for(;e; e--){
        uart_send((temp/exp(10, e))%10+'0');
    }
    uart_send('0'+(temp%10));
}