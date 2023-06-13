#include "aux_reg.h"
#include "mini_uart.h"
#include "timer.h"

void enable_interrupt()
{
	asm volatile("msr	DAIFClr,0xF");		//DAIF : Interrupt Mask Bits
	return;
}

void disable_interrupt()
{
	asm volatile("msr	DAIFSet,0xF");
	return;
}

static char write_buffer[1000];
static char read_buffer[1000];
static int write_front = 0;
static int write_back  = 0;
static int read_front  = 0;
static int read_back   = 0;

void TX_interrupt_handler()							//only when write_buffer have data -> TX interrupt invoke
{
	if(write_front != write_back)					//FIFO size is limited -> write_buffer may still have data
	{
		*AUX_MU_IO_REG = write_buffer[write_front];
		write_front++;
		return;
	}
	*AUX_MU_IER_REG &= 0x1;							//write_buffer no data -> diable TX interrupt , keep RX status
	return;
}

void RX_interrupt_handler()
{
	read_buffer[read_back] = (char)(*AUX_MU_IO_REG);
	read_back++;
	if(read_back > 999)
	{
		read_back = 0;
	}
	return;
}

void mini_uart_handler()
{
	if(*AUX_MU_IIR_REG & 0x2)		//bit2:1 (01) : Transmit holding register empty (FIFO empty)
	{
		TX_interrupt_handler();		//read write_buffer -> AUX_MU_IO_REG
	}
	else if(*AUX_MU_IIR_REG & 0x4)	//bit2:1 (10) Receiver holds valid byte (FIFO hold at least 1 symbol)
	{
		RX_interrupt_handler();		//AUX_MU_IO_REG -> read_buffer
	}
	return;
}

void EL0_core_timer_handler()
{
	asm volatile
	(
		"mrs     x0,CNTFRQ_EL0;"
		"asr	 x0,x0,5;"
    	"msr     CNTP_TVAL_EL0,x0;"		//set new timeout
	);
	schedule();							//use timer interrupt to switch_to next thread (RR)
	//print_time();
	return;
}

void EL1_core_timer_handler()
{
	asm volatile
	(
		"mrs     x0,CNTFRQ_EL0;"
		"asr     x0,x0,5;"
    	"msr     CNTP_TVAL_EL0,x0;"    //set new timeout , plus 1 sec
	);
	schedule();
	//print_time();
	//one_sec_pass();
	return;
}

void EL1_check_interrupt_source()
{
	if(*CORE0_INTERRUPT_SOURCE & 0x00000002)		//bit 1  : CNTPNSIRQ interrupt
	{
		//uart_send_string("async EL1 timer interrupt\r\n");
		EL1_core_timer_handler();		
	}	
	else if(*GPU_PENDING1_REGISTER & 0x20000000)	//bit 29 : AUX intterupt (uart1_interrupt)
	{
		mini_uart_handler();
	}
	return;
}

void EL0_check_interrupt_source()
{
	if(*CORE0_INTERRUPT_SOURCE & 0x00000002)	//bit 1  : CNTPNSIRQ interrupt
	{
		//uart_send_string("async EL0 timer interrupt\r\n");
		EL0_core_timer_handler();		
	}	
	return;
}

void mini_uart_enable()
{
	*AUX_MU_IER_REG = 0x1;			//enable RX interrupt
	*IRQS1 = 0x20000000;
	return;
}

void send_async_string(char* str)
{
	write_front = 0;				//clean write_buffer(fake)
	write_back = 0;
	for(int i=0;str[i] != '\0';i++)
	{
		write_buffer[write_back] = str[i];
		write_back++;
	}
	*AUX_MU_IER_REG |= 0x2;			//enable TX interrupt
	return;
}

void recv_async_string()
{
	*AUX_MU_IER_REG &= 0x2;			//disable RX interrupt , keep TX interrupt(when need to print/use AUX_MU_IO_REG , should close RX interrupt)
	char str[100];
	int i;
	for(i=0;read_front != read_back;i++)
	{
		str[i] = read_buffer[read_front];
		read_front++;
		if(read_front > 999)
		{
			read_front = 0;
		}
	}
	str[i] = '\0';
	read_front = 0;					//clean read_buffer(fake)
	read_back = 0;
	uart_send_string("recv -> ");
	uart_send_string(str);
	uart_send_string("\r\n");
	return;
}
