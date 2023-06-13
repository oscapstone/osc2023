#include "utils.h"
#include "printf.h"
#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"
#include "peripherals/exception.h"

void uart_send ( char c )
{
	if (c == '\n') uart_send('\r');

    while (1) {
		if (get32(AUX_MU_LSR_REG)&0x20) 
			break;
	}
    
	put32(AUX_MU_IO_REG,c);
}

char uart_recv ( void )
{
	while(1) {
		if (get32(AUX_MU_LSR_REG)&0x01) 
			break;
	}
	return (get32(AUX_MU_IO_REG)&0xFF);
}

void uart_send_string ( char* str )
{
	for (int i = 0; str[i] != '\0'; i++) {
		uart_send((char)str[i]);
	}
}

void printf(char *fmt, ...) {
    char temp[128];
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    vsprintf(temp,fmt,args);
    uart_send_string(temp);
}

void uart_init ( void )
{
    unsigned int selector;

    selector = get32(GPFSEL1);
    selector &= ~(7<<12);                   // clean gpio14
    selector |= 2<<12;                      // set alt5 for gpio14
    selector &= ~(7<<15);                   // clean gpio15
    selector |= 2<<15;                      // set alt5 for gpio 15
    put32(GPFSEL1,selector);

    put32(GPPUD,0);
    delay(150);
    put32(GPPUDCLK0,(1<<14)|(1<<15));
    delay(150);
    put32(GPPUDCLK0,0);

    put32(AUX_ENABLES,1);                   //Enable mini uart (this also enables access to its registers)
    put32(AUX_MU_CNTL_REG,0);               //Disable auto flow control and disable receiver and transmitter (for now)
    put32(AUX_MU_IER_REG,0);                //Enable receive and disable transmit interrupts
    put32(AUX_MU_LCR_REG,3);                //Enable 8 bit mode
    put32(AUX_MU_MCR_REG,0);                //Set RTS line to be always high
    put32(AUX_MU_BAUD_REG,270);             //Set baud rate to 115200
    put32(AUX_MU_IIR_REG,6);                //Interrupt identify no fifo
    put32(AUX_MU_CNTL_REG,3);               //Finally, enable transmitter and receiver

}
