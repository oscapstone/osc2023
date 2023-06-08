#include "peripherals/mini_uart.h"
#include "peripherals/irq.h"
#include "peripherals/gpio.h"
#include "string.h"


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


/*Display a string to console*/

//void uart_send_string(char *s) {
//    while(*s) {
//        /* convert newline to carriage return + newline */
//        if(*s=='\n') uart_send('\r');
//        uart_send(*s++);
//    }
//}



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

/* implement uart interrupt (asynchronous) EL1h_irq */

char uart_tx_buffer[VSPRINT_MAX_BUF_SIZE]={}; //buffer for uart to monitor
unsigned int uart_tx_buffer_widx = 0;  //write index
unsigned int uart_tx_buffer_ridx = 0;  //read index
char uart_rx_buffer[VSPRINT_MAX_BUF_SIZE]={}; //buffer for keyboard to uart
unsigned int uart_rx_buffer_widx = 0;
unsigned int uart_rx_buffer_ridx = 0;

// AUX_MU_IER_REG -> BCM2837-ARM-Peripherals.pdf - Pg.12
void uart_interrupt_enable(){
    *AUX_MU_IER |=1;  // enable read interrupt (第0位設為1)
    *AUX_MU_IER |=2;  // enable write interrupt（第1位設為1） 
    *ENABLE_IRQS_1  |= 1 << 29;    // Pg.112 （第29位設為1) 
}

void uart_interrupt_disable(){
    *AUX_MU_IER &= ~(1);  // disable read interrupt
    *AUX_MU_IER &= ~(2);  // disable write interrupt
}


void uart_r_irq_handler(){ //read
    if((uart_rx_buffer_widx + 1) % VSPRINT_MAX_BUF_SIZE == uart_rx_buffer_ridx) //check if buffer is full
    {
        *AUX_MU_IER &= ~(1);  // disable read interrupt
        return;
    }
    uart_rx_buffer[uart_rx_buffer_widx++] = uart_recv(); 
    if(uart_rx_buffer_widx>=VSPRINT_MAX_BUF_SIZE) uart_rx_buffer_widx=0;
    *AUX_MU_IER |=1; //keep putting in buffer
}

void uart_w_irq_handler(){ //write
    if(uart_tx_buffer_ridx == uart_tx_buffer_widx) //if bufer is empty
    {
        *AUX_MU_IER &= ~(2);  // disable write interrupt
        return; 
    }
    //if buffer is not empty:
    uart_send(uart_tx_buffer[uart_tx_buffer_ridx++]);
    if(uart_tx_buffer_ridx>=VSPRINT_MAX_BUF_SIZE) uart_tx_buffer_ridx=0;
    *AUX_MU_IER |=2;  // enable write interrupt
}



// uart_async_getc read from buffer
// uart_r_irq_handler write to buffer then output
char uart_async_getc() {
    *AUX_MU_IER |=1; // enable read interrupt
    // do while if buffer empty
    while (uart_rx_buffer_ridx == uart_rx_buffer_widx) *AUX_MU_IER |=1; // enable read interrupt
    el1_interrupt_disable();
    char r = uart_rx_buffer[uart_rx_buffer_ridx++];
    if (uart_rx_buffer_ridx >= VSPRINT_MAX_BUF_SIZE) uart_rx_buffer_ridx = 0;
    el1_interrupt_enable();
    return r;
}


// uart_async_putc writes to buffer
// uart_w_irq_handler read from buffer then output
void uart_async_putc(char c) {
    // if buffer full, wait for uart_w_irq_handler
    while( (uart_tx_buffer_widx + 1) % VSPRINT_MAX_BUF_SIZE == uart_tx_buffer_ridx )  *AUX_MU_IER |=2;  // enable write interrupt
    el1_interrupt_disable();
    uart_tx_buffer[uart_tx_buffer_widx++] = c;
    if(uart_tx_buffer_widx >= VSPRINT_MAX_BUF_SIZE) uart_tx_buffer_widx=0;  // cycle pointer
    el1_interrupt_enable();
    *AUX_MU_IER |=2;  // enable write interrupt
}

void  uart_send_string(char* fmt, ...) {
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
    //return count;
}
