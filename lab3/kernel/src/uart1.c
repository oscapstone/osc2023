#include "bcm2837/rpi_gpio.h"
#include "bcm2837/rpi_uart1.h"
#include "uart1.h"
#include "u_string.h"

#define IRQS1  ((volatile unsigned int*)(0x3f00b210))

//implement first in first out buffer with a read index and a write index
char uart_tx_buffer[VSPRINT_MAX_BUF_SIZE]={};
unsigned int uart_tx_buffer_widx = 0;  //write index
unsigned int uart_tx_buffer_ridx = 0;  //read index
char uart_rx_buffer[VSPRINT_MAX_BUF_SIZE]={};
unsigned int uart_rx_buffer_widx = 0;
unsigned int uart_rx_buffer_ridx = 0;

void uart_init()
{
    register unsigned int r;

    /* initialize UART */
    *AUX_ENABLES     |= 1;       // enable UART1
    *AUX_MU_CNTL_REG  = 0;       // disable TX/RX

    /* configure UART */
    *AUX_MU_IER_REG   = 0;       // disable interrupt
    *AUX_MU_LCR_REG   = 3;       // 8 bit data size
    *AUX_MU_MCR_REG   = 0;       // disable flow control
    *AUX_MU_BAUD_REG  = 270;     // 115200 baud rate
    *AUX_MU_IIR_REG   = 6;       // disable FIFO

    /* map UART1 to GPIO pins */
    r = *GPFSEL1;
    r &= ~(7<<12);               // clean gpio14
    r |= 2<<12;                  // set gpio14 to alt5
    r &= ~(7<<15);               // clean gpio15
    r |= 2<<15;                  // set gpio15 to alt5
    *GPFSEL1 = r;

    /* enable pin 14, 15 - ref: Page 101 */
    *GPPUD = 0;
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = (1<<14)|(1<<15);
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = 0;

    *AUX_MU_CNTL_REG = 3;      // enable TX/RX
}

char uart_recv() {
    char r;
    while(!(*AUX_MU_LSR_REG & 0x01)){};
    r = (char)(*AUX_MU_IO_REG);
    return r=='\r'?'\n':r;
}

void uart_send(char c) {
    while(!(*AUX_MU_LSR_REG & 0x20)){};
    *AUX_MU_IO_REG = c;
}

void uart_2hex(unsigned int d) {
    unsigned int n;
    int c;
    for(c=28;c>=0;c-=4) {
        n=(d>>c)&0xF;
        n+=n>9?0x37:0x30;
        uart_send(n);
    }
}

int  uart_sendline(char* fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    char buf[VSPRINT_MAX_BUF_SIZE];

    char *str = (char*)buf;
    int count = vsprintf(str,fmt,args);

    while(*str) {
        if(*str=='\n')
            uart_send('\r');
        uart_send(*str++);
    }
    __builtin_va_end(args);
    return count;
}

char uart_async_getc() {
    // while buffer empty
    // enable read interrupt to get some input into buffer
    while (uart_rx_buffer_ridx == uart_rx_buffer_widx) *AUX_MU_IER_REG |=1; // enable read interrupt
    char r = uart_rx_buffer[uart_rx_buffer_ridx++];
    return r;
}

void uart_async_putc(char c) {
    while( (uart_tx_buffer_widx + 1) % VSPRINT_MAX_BUF_SIZE == uart_tx_buffer_ridx ) // full buffer wait
    {
        // start asynchronous transfer 
        *AUX_MU_IER_REG |=2;  // enable write interrupt
    }
    uart_tx_buffer[uart_tx_buffer_widx++] = c;
    if(uart_tx_buffer_widx >= VSPRINT_MAX_BUF_SIZE)uart_tx_buffer_widx=0;  // cycle pointer
    // start asynchronous transfer
    *AUX_MU_IER_REG |=2;  // enable write interrupt
}

int  uart_puts(char* fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    char buf[VSPRINT_MAX_BUF_SIZE];

    char *str = (char*)buf;
    int count = vsprintf(str,fmt,args);

    while(*str) {
        if(*str=='\n')
            uart_async_putc('\r');
        uart_async_putc(*str++);
    }
    __builtin_va_end(args);
    return count;
}

//https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf p13
/*
 AUX_MU_IIR
 on read bits[2:1] :
 00 : No interrupts
 01 : Transmit holding register empty
 10 : Receiver holds valid byte
 11: <Not possible> 
*/

// buffer read, write
void uart_interrupt_handler(){
    if(*AUX_MU_IIR_REG & (1<<1)) //on write
    {
        if(uart_tx_buffer_ridx == uart_tx_buffer_widx)
        {
            *AUX_MU_IER_REG &= ~(2);  // disable write interrupt
            return;  // buffer empty
        }
        uart_send(uart_tx_buffer[uart_tx_buffer_ridx++]);
        if(uart_tx_buffer_ridx>=VSPRINT_MAX_BUF_SIZE) uart_tx_buffer_ridx=0;
    }
    else if(*AUX_MU_IIR_REG & (2<<1)) //on read
    {
        if((uart_rx_buffer_widx + 1) % VSPRINT_MAX_BUF_SIZE == uart_rx_buffer_ridx)
        {
            *AUX_MU_IER_REG &= ~(1);  // disable read interrupt
            return;
        }
        uart_rx_buffer[uart_rx_buffer_widx++] = uart_recv();
        uart_send(uart_rx_buffer[uart_rx_buffer_widx-1]);
        if(uart_rx_buffer_widx>=VSPRINT_MAX_BUF_SIZE) uart_rx_buffer_widx=0;
    }else
    {
        uart_puts("uart_interrupt_handler error!!\n");
    }

}

void uart_interrupt_enable(){
    *AUX_MU_IER_REG |=1;  // enable read interrupt
    *AUX_MU_IER_REG |=2;  // enable write interrupt
    *IRQS1 |= 1 << 29;
}

void uart_interrupt_disable(){
    *AUX_MU_IER_REG &= ~(1);  // disable read interrupt
    *AUX_MU_IER_REG &= ~(2);  // disable write interrupt
}

