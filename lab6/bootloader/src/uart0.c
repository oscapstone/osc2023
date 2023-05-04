#include "uart0.h"
#include "mbox.h"
#include "gpio.h"

void uart0_init()
{	
	*UART0_CR = 0;				//turn off UART0

	//set up clock frequency
	unsigned int mbox[9];
	mbox[0] = 9*4;
	mbox[1] = REQUEST_CODE;
	mbox[2] = SET_CLKRATE;
	mbox[3] = 12;
	mbox[4] = 8;				//value buffer
	mbox[5] = 2;				//uart clock
	mbox[6] = 4000000;			//4Mhz
	mbox[7] = 0;				//clear turbo
	mbox[8] = END_TAG;
	mbox_call(mbox,8);

	//Enable GPIO
	register unsigned int selector;
	selector = *GPFSEL1;
	selector &= ~((7<<12)|(7<<15));
	selector |= (4<<12)|(4<<15);	//alt0: TXD0 , RXD0 (100:function 0)
	*GPFSEL1 = selector;

	*GPPUD = 0;
	register unsigned int t;
	t = 150;
	while(t--)
	{
		asm volatile("nop");
	}
	*GPPUDCLK0 = (1<<14) | (1<<15);
	t = 150;
	while(t--)
	{
		asm volatile("nop");
	}
	*GPPUDCLK0 = 0;

	*UART0_ICR = 0x7FF;		//UARTICR register is the interrupt clear register , bit15:11(reserved) , bit10:0(1:interrupt clear , 0:no effect)
	*UART0_IBRD = 0x2;		//IBRD: integer part of the baud rate
	*UART0_FBRD = 0xB;		//FBRD: fractional part of the baud rate
	//programmed divisor(integer) 0x2 , progammed divisor(fraction) 0xB --> required bit rate in bps 115200
	*UART0_LCRH = 0x7<<4;	//line control register , bit4:enable FIFO , bit5~6: 11=8bit
	*UART0_CR = 0x301;		//control register , bit9: (Rx)receive enable , bit8: (Tx)transmit enable , bit0: UART enable
	return;
}

void uart0_send(unsigned int c)
{
	while(1)
	{
		if(!(*UART0_FR & 0x20))		//bit5: set if transmit FIFO is full -> can't send
		{
			break;
		}
	}
	*UART0_DR = c;
	return;
}

char uart0_recv()
{
	while(1)
	{
		if(!(*UART0_FR & 0x10))		//bit4: set if receive FIFO is empty -> no data
		{
			break;
		}
	}
	return (char)(*UART0_DR);
}

void uart0_send_string(char *str)
{
	for(int i=0;str[i] != '\0';i++)
	{
		uart0_send((char)str[i]);
	}
	return;
}
