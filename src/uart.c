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
#define ENABLE_IRQS_1   ((volatile unsigned int*)(MMIO_BASE+0x0000B210))
#define DISABLE_IRQS_1  ((volatile unsigned int*)(MMIO_BASE+0x0000B21C))

void uart_init()
{
    register unsigned int r;

    /* initialize UART */
    *AUX_ENABLE |=1;       // enable UART1, AUX mini uart
    *AUX_MU_CNTL = 0;
    *AUX_MU_LCR = 3;       // 8 bits
    *AUX_MU_MCR = 0;
    *AUX_MU_IER = 0;
    *AUX_MU_IIR = 0x6;    // disable interrupts
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

/**
 * Send a character
 */
void uart_send(unsigned int c) {
    /* wait until we can send */
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x20));
    /* write the character to the buffer */
    *AUX_MU_IO=c;
}

/**
 * Receive a character
 */
char uart_getc() {
    char r;
    /* wait until something is in the buffer */
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x01));
    /* read it and return */
    r=(char)(*AUX_MU_IO);
    /* convert carriage return to newline */
    return r=='\r'?'\n':r;
}
char uart_recv(){
    while (1) {
        if ((*AUX_MU_LSR)&0x01) break;
    }
    return (*AUX_MU_IO)&0xFF;
}

void uart_getline(char* buf, int maxlen){
    int count = 0;
    while(count < maxlen - 1){
        char recv = uart_getc();
        uart_send(recv);
        if(recv == '\n' || recv == '\r' || recv == '\0'){
            break;
        }
        buf[count] = recv;
        count ++;
    }    
    buf[count] = '\0';
    return;
}

/**
 * Display a string
 */
void uart_puts(char *s) {
    while(*s) {
        /* convert newline to carriage return + newline */
        if(*s=='\n')
            uart_send('\r');
        uart_send(*s++);
    }
}

void uart_hex(unsigned int d){
    unsigned int n;
    int c;
    uart_puts("0x");
    for (c = 28; c >= 0; c -= 4) {
        // get highest tetrad
        n = (d >> c) & 0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n += n > 9 ? 0x37 : 0x30;
        uart_send(n);
    }
}

int uart_get_int(){
    int num = 0;
    for(int i = 0; i < 4; i++){
        char temp = uart_recv();
        num = num << 8;
        num += (int)temp;
    }
    return num;
}

void uart_send_int(int number){
    uart_send((char)((number >> 24) & 0xFF));
    uart_send((char)((number >> 16) & 0xFF));
    uart_send((char)((number >> 8) & 0xFF));
    uart_send((char)(number & 0xFF));
}

char async_recv_buf[256];
char async_send_buf[256];
int write_head, write_end;
int read_head, read_end;

void enable_uart_irq(){
    *ENABLE_IRQS_1 = (1 << 29);
}

void disable_uart_irq(){
    *DISABLE_IRQS_1 = (1 << 29);
}

void async_uart_recv(){
    enable_uart_irq(); 
}
void async_uart_handle(){
    // disable_uart_irq();
    unsigned long  iir = *AUX_MU_IIR;
    // has interrupt pending
    if ((iir & 1) == 0) {
      //send
      if (iir & 2) {
        while (1) {
            if ((*AUX_MU_LSR)&0x20) break;
        }
        char c = async_send_buf[write_head];
        write_head++;
        *AUX_MU_IO = c;
        if(write_head == 256) write_head = 0;
        (*AUX_MU_IER) &= ~0x02;
      } 
      // read 
      else if (iir & 4) {
        while (1) {
            if ((*AUX_MU_LSR)&0x01) break;
        }
        char c =  (*AUX_MU_IO)&0xFF;
        async_recv_buf[read_end] = c;
        read_end ++;
        // reset if read till the end of buffer
        if(read_end == 256) read_end = 0;
      }
    }
    // enable_uart_irq();
}
char read_from_async_recv_buf(){
    // nothing to read 
    while(read_head == read_end){
        asm volatile("nop");
    }
    char c = async_recv_buf[read_head];
    read_head ++;
    if(read_head == 256) read_head = 0;
    return c;
}

void copy_to_async_send_buf(char c){
    async_send_buf[write_end] = c;
    write_end ++;
    if(write_end == 256) write_end = 0;
    // assert transmitter interrupt 
    (*AUX_MU_IER) |= 0x02;
}


void async_test(){
    *AUX_MU_IER = 1;
    enable_uart_irq();

    while(1){
        char c = read_from_async_recv_buf();
        copy_to_async_send_buf(c);
        if(c == '\r') copy_to_async_send_buf('\n');
    }

    disable_uart_irq();
    *AUX_MU_IER = 0;
    uart_puts("END\n\r");
}