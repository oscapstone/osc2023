
#include "gpio.h"
#include "uart.h"
#include "string.h"
#include "exception.h"
#include "shell.h"


#define IRQS1  ((volatile unsigned int*)(0x3f00b210)) // BCM2835, section-1.2.3

//Implement FIFO buffer with a read index and a write index
char uart_tx_buffer[VSPRINT_MAX_BUF_SIZE]={}; //uart transmit buffer
unsigned int uart_tx_buffer_widx = 0;         // read index -> head of list
unsigned int uart_tx_buffer_ridx = 0;         // write index -> tail of list
char uart_rx_buffer[VSPRINT_MAX_BUF_SIZE]={}; //uart write buffer
unsigned int uart_rx_buffer_widx = 0;
unsigned int uart_rx_buffer_ridx = 0;




/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart_init()
{
    register unsigned int reg;

    /* initialize UART */
    *AUX_ENABLE     |= 1;       /* enable mini UART */
    *AUX_MU_CNTL     = 0;       /* Disable transmitter and receiver during configuration. */

    *AUX_MU_IER      = 0;       /* Disable interrupt */
    *AUX_MU_LCR      = 3;       /* Set the data size to 8 bit. */
    *AUX_MU_MCR      = 0;       /* Don’t need auto flow control. */
    *AUX_MU_BAUD     = 270;     /* 115200 baud */
    *AUX_MU_IIR      = 6;       /* No FIFO */
    // *AUX_MU_IIR      = 0xc6;       /* No FIFO */

    /* map UART1 to GPIO pins */
    reg = *GPFSEL1;
    reg &= ~((7<<12)|(7<<15));  /* address of gpio 14, 15 */
    reg |=   (2<<12)|(2<<15);   /* set to alt5 */

    *GPFSEL1 = reg;

    *GPPUD = 0;                 /* enable gpio 14 and 15 */
    reg=150;
    while ( reg-- )
    { 
        asm volatile("nop"); 
    }
    
    *GPPUDCLK0 = (1<<14)|(1<<15);
    reg=150; 
    while ( reg-- )
    {
        asm volatile("nop");
    }
    
    *GPPUDCLK0 = 0;             /* flush GPIO setup */

    *AUX_MU_CNTL = 3;           // Enable the transmitter and receiver.
}

/**
 * Send a character
 */
void uart_send(unsigned int c)
{
    /* Wait until we can send */
    do {
        
        asm volatile("nop");

    } while( ! ( *AUX_MU_LSR&0x20 ));
    
    /* write the character to the buffer */   
    *AUX_MU_IO = c;

    if ( c == '\n' ) 
    {
        do {
            
            asm volatile("nop");

        } while( ! ( *AUX_MU_LSR&0x20 ));
        
        *AUX_MU_IO = '\r';
    }
}

/**
 * Receive a character
 */
char uart_getc() {

    char r;
    
    /* wait until something is in the buffer */
    do{
        
        asm volatile("nop");
        
    } while ( ! ( *AUX_MU_LSR&0x01 ) );

    /* read it and return */
    r = ( char )( *AUX_MU_IO );

    /* convert carrige return to newline */
    return r == '\r' ? '\n' : r;
}

/**
 * Display a string
 */
void uart_puts(char *s)
{
    while( *s )
    {
        uart_send(*s++);
    }
}



int uart_get_int(){
    int res = 0;
    char c;
    while(1){
        c = uart_getc();
        if(c == '\0' || c == '\n')
            break;
        uart_send(c);
        res = res * 10 + (c - '0');
    }
    return res;
}


void uart_puts_bySize(char *s, int size){
    for(int i = 0; i < size ;++i){
        if(*s == '\n') uart_send('\r');
        uart_send(*s++);
    }
}


void uart_put_int(unsigned long num){
    if(num == 0) uart_send('0');
    else{
        if(num > 10) uart_put_int(num / 10);
        uart_send(num % 10 + '0');
    }
}




int  uart_printf(char* fmt, ...) {
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
    *AUX_MU_IER |=1; // Enable read interrupt
    // While buffer empty
    // Enable read interrupt to get some input into buffer
    while (uart_rx_buffer_ridx == uart_rx_buffer_widx) *AUX_MU_IER |=1; // enable read interrupt
    el1_interrupt_disable();  //Disable EL1 interrupts to prevent race conditions while reading characters.
    char r = uart_rx_buffer[uart_rx_buffer_ridx++]; //Read a character from the receive buffer and move the read pointer to the next position
    if (uart_rx_buffer_ridx >= VSPRINT_MAX_BUF_SIZE) uart_rx_buffer_ridx = 0; //If the read pointer is out of bounds of the buffer, it is set to 0 to function as a circular buffer.
    el1_interrupt_enable(); // Enable EL1 interrupt
    return r;
}

char uart_recv() {
     char r;
     while(!(*AUX_MU_LSR & 0x01)){};
     r = (char)(*AUX_MU_IO);
     return r=='\r'?'\n':r;
 }


void uart_async_putc(char c) {
    while( (uart_tx_buffer_widx + 1) % VSPRINT_MAX_BUF_SIZE == uart_tx_buffer_ridx ) // Full buffer wait
    {
        // Start asynchronous transfer 
        *AUX_MU_IER |=2;  // Enable write interrupt
    }
    uart_tx_buffer[uart_tx_buffer_widx++] = c; //Write the character c into the send buffer and move the send pointer to the next position.
    if(uart_tx_buffer_widx >= VSPRINT_MAX_BUF_SIZE)uart_tx_buffer_widx=0;  // cycle pointer
    // start asynchronous transfer
    *AUX_MU_IER |=2;  // enable write interrupt
}


int  uart_async_puts(char* fmt, ...) {
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



int uart_async_printf(char *fmt, ...)
{
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    char buf[MAX_BUFFER_LEN];
    // we don't have memory allocation yet, so we
    // simply place our string after our code
    char *s = (char *)buf;
    // use sprintf to format our string
    int count = vsprintf(s, fmt, args);
    // print out as usual
    while (*s)
    {
        uart_async_putc(*s++);
    }
    __builtin_va_end(args);
    return count;
}



// AUX_MU_IER -> BCM2837-ARM-Peripherals.pdf - Pg.12
void uart_interrupt_enable(){
    *AUX_MU_IER |=1;  // enable read interrupt, AUX_MU_IER[0] for receive interrupts.
    *AUX_MU_IER |=2;  // enable write interrupt, AUX_MU_IER[1] for transmit interrupts.
    *IRQS1 |= 1 << 29;  // Pg.113, ARM peripherals interrupts table, unmask AUX_INT. 
}

void uart_interrupt_disable(){
    *AUX_MU_IER &= ~(1);  // disable read interrupt
    *AUX_MU_IER &= ~(2);  // disable write interrupt
}


void uart_r_irq_handler(){
    if((uart_rx_buffer_widx + 1) % VSPRINT_MAX_BUF_SIZE == uart_rx_buffer_ridx)
    {
        *AUX_MU_IER &= ~(1);  // disable read interrupt
        return;
    }

    uart_rx_buffer[uart_rx_buffer_widx++] = uart_recv();
    uart_send(uart_rx_buffer[uart_rx_buffer_widx-1]);
    if(uart_rx_buffer_widx >= VSPRINT_MAX_BUF_SIZE) uart_rx_buffer_widx=0;

    *AUX_MU_IER |=1; // enable read interrupt
}

void uart_w_irq_handler(){
    if(uart_tx_buffer_ridx == uart_tx_buffer_widx)
    {
        *AUX_MU_IER &= ~(2);  // disable write interrupt
        return;  // buffer empty
    }

    uart_send(uart_tx_buffer[uart_tx_buffer_ridx++]);
    if(uart_tx_buffer_ridx>=VSPRINT_MAX_BUF_SIZE) uart_tx_buffer_ridx=0;

    *AUX_MU_IER |=2;  // enable write interrupt
}