#include "interrupt.h"
#include "exception.h"
#include "uart.h"
#include "timer.h"

int core_timer_enable(void){
	init_timer_Q();
	asm volatile(
		"mov	x0, 	1;"
		"msr	cntp_ctl_el0, x0;"	// Enable timer
		"mrs	x0, 	cntfrq_el0;"	// Get count fequency
		"mov	x1,	2;"
		"mul	x0, 	x0, x1;"
		"msr	cntp_tval_el0, x0;"	// Set expire time
		"mov	x0,	2;"
		"ldr	x1, 	=0x40000040;"	// Timer interrupt
		"str	w0,	[x1];"		// Unmask the timer interrupt
	);
	uart_puts(" CORE TIMER INITIAL\n");
	return 0;
}

/**************************************************************************
 * Enable mini uart interrupt.
 * Need to enable AUX int (bit 29)
 * Need to connect the GPU IRQ to CORE0's IRQ
 *************************************************************************/
int mini_uart_interrupt_enable(void){
	*IRQS1 |= (1<<29);	// Encble aux int
	//*AUX_MU_IER = 0x1;	// Enable aux rx interrupt
	//*GPU_INT_ROUT = 0;	// GPU FIQ&IRQ -> CORE0 FIQ&IRQ
	return 0;
}

/**************************************************************************
 * Timer handler which handle the timer interrupt from current EL.
 *************************************************************************/
int core_timer_handler(void){
	//uart_puts(" Timer handler!\n");
	//uint64_t time, freq;
	//asm volatile(
	//	"mrs	%[time], cntpct_el0;"
	//	"mrs	%[freq], cntfrq_el0;"
	//	: [time] "=r" (time), [freq] "=r" (freq)
	//);
	//uart_puts("Current second: ");
	//uart_puthl(time / freq);
	//uart_puts("\n");
	timer_walk(1);
	asm volatile(
		"mrs	x0,	cntfrq_el0;" // Get the clock frequency
		"msr	cntp_tval_el0,	x0;"	// set tval = cval + x0
	);
	return 0;
}

/**************************************************************************
 * Timer handler which handle the timer interrupt from lower EL
 *************************************************************************/
int low_core_timer_handler(void){
	uart_puts(" Timer handler!\n");
	uint64_t time, freq;
	asm volatile(
		"mrs	%[time], cntpct_el0;"
		"mrs	%[freq], cntfrq_el0;"
		: [time] "=r" (time), [freq] "=r" (freq)
	);
	uart_puts("Current second: ");
	uart_puthl(time / freq);
	uart_puts("\n");
	//timer_walk(2);
	asm volatile(
		"mrs	x0,	cntfrq_el0;" // Get the clock frequency
		"mov	x1,	2;"
		"mul	x0, 	x0, x1;"
		"msr	cntp_tval_el0,	x0;"	// set tval = cval + x0
	);
	return 0;
}

/**************************************************************************
 * Function which get the value of CORE0's IRQ.
 * 
 * return: int32_t: the source ID of the interrupt.
 *************************************************************************/
static unsigned int get_core0_irq_source(void) {
	return *CORE0_IRQ_SOURCE;
}

/**************************************************************************
 * Clear the IRQ
 *************************************************************************/
static int clear_core0_irq_source(void) {
	*CORE0_IRQ_SOURCE = 0;
	return 0;
}

/**************************************************************************
 * uart handler. Assign the interrupt to the target service functions.
 *************************************************************************/
static int uart_handler(void){
	//uart_puts("uart_handler\n");
	//uart_puth(*AUX_MU_IIR);
	// Only care the 2:1 bit in this register.
	switch (*AUX_MU_IIR & 0x6){
	case 2:
		uart_transmit_handler();
		break;
	case 4:
		uart_receive_handler();
		break;
	case 0:
	default:
		uart_puts("Error\n");
		break;
	}
	return 0;
}

/**************************************************************************
 * Disable interrupt in current EL
 *************************************************************************/
int disable_int(void) {
	asm volatile(
		"msr	DAIFSet, 0xF;"
	);
	return 0;
}

/**************************************************************************
 * Enable interrupt in current EL
 *************************************************************************/
int enable_int(void) {
	asm volatile(
		"msr	DAIFClr, 0xf;"
	);
	mini_uart_interrupt_enable();
	return 0;
}

/**************************************************************************
 * IRQ handler which from current EL
 *************************************************************************/
int irq_handler(void){
	disable_int();
	//uint32_t source = get_core0_irq_source();
	//uart_puts("IRQ_HANDLER\n");
	//uart_puts("IRQ source: ");
	//uart_puth(get_core0_irq_source());
	//uart_puts("\n");
	//print_spsr_el1();
	//print_elr_el1();
	//print_esr_el1();

	//if(*IRQ_PEND_1 & (1<<29) && *AUX_MU_IIR & 0x1)
	if(*IRQ_PEND_1 & (1 << 29))
		uart_handler();
	else if(*CORE0_IRQ_SOURCE & 0x2)
		core_timer_handler();
	else
		exception_entry();

	//uart_puts("IRQ_HANDLER END\n");
	enable_int();
	return 0;
}

/**************************************************************************
 * IRQ handler which handles the interrupts from lower EL
 *************************************************************************/
int low_irq_handler(void){
	disable_int();
	//uint32_t source = get_core0_irq_source();
	//uart_puts("IRQ_HANDLER\n");
	//uart_puts("IRQ source: ");
	//uart_puth(get_core0_irq_source());
	//uart_puts("\n");
	//print_spsr_el1();
	//print_elr_el1();
	//print_esr_el1();

	//if(*IRQ_PEND_1 & (1<<29) && *AUX_MU_IIR & 0x1)
	if(*IRQ_PEND_1 & (1 << 29))
		uart_handler();
	else if(*CORE0_IRQ_SOURCE & 0x2)
		low_core_timer_handler();
	else
		exception_entry();

	//uart_puts("IRQ_HANDLER END\n");
	enable_int();
	return 0;
}
