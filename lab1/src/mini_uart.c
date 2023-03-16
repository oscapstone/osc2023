#include "gpio.h"
#include "mini_uart.h"

void delay(unsigned int clock){
    while (clock--){
        asm volatile("nop"); // insert loop
    }
}

void uart_init(){

    *AUX_ENABLE = 1;           //enable mini UART (mini uart can be accessed)
    *AUX_MU_CNTL_REG = 0;      //Disable transmitter and receiver during configuration.
    *AUX_MU_IER_REG = 0;       //Disable interrupt because currently you don’t need interrupt.
    *AUX_MU_LCR_REG = 3;       //Set the data size to 8 bit.
    *AUX_MU_MCR_REG =0;        //Don’t need auto flow control.
    *AUX_MU_BAUD_REG = 270;    //Set baud rate to 115200
    *AUX_MU_IIR_REG = 6;       // No FIFO
    *AUX_MU_CNTL_REG =3;       //Enable the transmitter and receiver.

    /////map mini_uart to gpio pins////
    unsigned int reg;
    reg = *GPFSEL1;
    reg &= ~((7<<12)|(7<<15)); //clean gpio14,15
    reg |= ((2<<12)|(2<<15)); //set alt5
    *GPFSEL1 = reg;

    *GPPUD = 0; //enable gpio14,15
    delay(200); // for set
    *GPPUDCLK0 = ((1 << 14) | (1 << 15));
    delay(200);
    *GPPUDCLK0 = 0;
}

/// send a character
void uart_send(char c){
    while (!(*(AUX_MU_LSR_REG)&0x20)){
    }
    /*Reserved, write zero, 
    read as don’t care Some of these bits have functions in a 16550 compatible UART 
    but are ignored here*/

    *AUX_MU_IO_REG = c;
}

void uart_hex(unsigned int d){
    unsigned int n;
    int c;
    uart_display("0x");

    
    for(c=28;c>=0;c-=4){
        //get highest tetrad
        n=(d>>c)&0xF;
        //0-9 => '0'-'9', 10-15 => 'A'-'F'
        n+=n>9?0x37:0x30;
        uart_send(n);
    }
}



/// receive
char uart_recv(){
    while (!(*(AUX_MU_LSR_REG)&0x01)){        
    }
    /*This bit is set if there was a receiver overrun. That is:
    one or more characters arrived whilst the receive
    FIFO was full. The newly arrived charters have been
    discarded. This bit is cleared each time this register is
    read. To do a non-destructive read of this overrun bit
    use the Mini Uart Extra Status register*/
    char r = *(AUX_MU_IO_REG);
    return r == '\r' ? '\n' : r;
}

void uart_display(char *s){
    while(*s){
        uart_send(*s++);
    }
}
