#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"
#include "peripherals/mailbox.h"
#include "utils.h"
#include "exc.h"
#define BUF_SIZE 64

/* mailbox message buffer */
volatile unsigned int  __attribute__((aligned(16))) mbox[36];
static char rx_buf[BUF_SIZE];
static char tx_buf[BUF_SIZE];
int rx_index;
int tx_index;

/**
 * initialize UART
 */
void uart_init ( void ) {
    register unsigned int r;

    *AUX_ENABLES |=1;       // enable UART1, AUX mini uart
    *AUX_MU_CNTL_REG = 0;
    *AUX_MU_IER_REG = 0;
    *AUX_MU_LCR_REG = 3;       // 8 bits
    *AUX_MU_MCR_REG = 0;
    *AUX_MU_BAUD_REG = 270;    // 115200 baud
    *AUX_MU_IIR_REG = 6;
    *AUX_MU_CNTL_REG = 3;
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
    *AUX_MU_CNTL_REG = 3;      // enable Tx, Rx
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

/**
 * Asynchronous IO send by TX
 */
void uart_async_send(char * str, int len) {
	if (len > BUF_SIZE-1 || len < 0) {
		uart_puts("Asynchronous send size invalid.\n");
	} else {
		for (int i = 0; i < len; i++){
			tx_buf[i] = str[i];
		}
		tx_buf[len] = 0;
		// Enable Tx
		enable_tx();
	}
}

/**
 * Asynchronous IO receive by RX
 */
void uart_async_getc(char * str, int len) {
	// disable Rx
	if (len < 0 || len > BUF_SIZE){
		uart_puts("Asynchronous get size invalid.\n");
	} else {
	
		*AUX_MU_IER_REG &= 0x2;
		for(int i = 0; i<len; i++){
			str[i] = rx_buf[i];
		}
	}
	buf_clear(rx_buf, BUF_SIZE);
}

/**
 * Enable_tx, put content in tx buffer into AUX_MU_IO char by char
 */
void uart_transmit_handler(){
	if (tx_buf[tx_index]==0){
		tx_index = 0;
		disable_tx();
		return;
	} else {	
		*AUX_MU_IO_REG = tx_buf[tx_index++];
		enable_tx();
	}
}

/**
 * Enable_rx, put content in AUX_MU_IO into rx buffer
 */
void uart_receive_handler(){
	if (rx_index >= BUF_SIZE) {
		rx_index = 0;
		disable_rx();
	} else{
		rx_buf[rx_index] = (char)(*AUX_MU_IO_REG);
		rx_index++;
	}
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

/**
 * Display a string. Solve the problem: Auto stop when encounter 0x0
 */
void uart_puts_l(char *s, int l) {
	for (int i = 0; i<l; i++) {
        if(s[i]=='\n')
            uart_send('\r');
		if(s[i]=='\0')
            uart_send(' ');
        else
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

void uart_hexlong(unsigned long d) {
    unsigned int n;
    int c;
    for(c=60;c>=0;c-=4) {
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
}

void uart_ulong(unsigned long i) {
	    char c[20];
	    if (i == 0) {
			uart_send('0');
			return;
		}
		int digits = -1;
		while (i != 0) {
			c[++digits] = '0' + i % 10;
			i /= 10;
		}
		for (; digits >= 0; --digits) {
			uart_send(c[digits]);
		}
}

void set(long addr, unsigned int value) {
    volatile unsigned int* point = (unsigned int*)addr;
    *point = value;
}

void reset(int tick) {                 // reboot after watchdog timer expire
    set(PM_RSTC, PM_PASSWORD | 0x20);  // full reset
    set(PM_WDOG, PM_PASSWORD | tick);  // number of watchdog tick
}

void cancel_reset() {
    set(PM_RSTC, PM_PASSWORD | 0);  // full reset
    set(PM_WDOG, PM_PASSWORD | 0);  // number of watchdog tick
}

void mbox_call(unsigned char ch)
{

    // get the board's unique serial number with a mailbox call
    mbox[0] = 7*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = GET_BOARD_REVISION;   // get serial number command
    mbox[3] = 4;                    // buffer size
    mbox[4] = TAG_REQUEST_CODE;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = END_TAG;

    unsigned int r = (((unsigned int)((unsigned long)&mbox)&~0xF) | (ch&0xF));
    do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_FULL);
    *MBOX_WRITE = r;
    // while(1) {
    //     do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_EMPTY);
    // }
    uart_puts("board revision: ");
    uart_hex(mbox[5]);
    uart_puts("\n");

    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = GET_ARM_MEMORY;       // get serial number command
    mbox[3] = 8;                    // buffer size
    mbox[4] = TAG_REQUEST_CODE;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = 0;
    mbox[7] = END_TAG;

    r = (((unsigned int)((unsigned long)&mbox)&~0xF) | (ch&0xF));
    do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_FULL);
    *MBOX_WRITE = r;
    // while(1) {
    //     do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_EMPTY);
    // }
    uart_puts("ARM memory base address: ");
    uart_hex(mbox[5]);
    uart_puts("\n");

    uart_puts("ARM memory size: ");
    uart_hex(mbox[6]);
    uart_puts("\n");
}
