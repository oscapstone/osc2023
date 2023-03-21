#include "interrupt.h"
#include "uart.h"

int core_timer_enable(void){
	asm volatile(
		"mov	x0, 	1;"
		"msr	cntp_ctl_el0, x0;"	// Enable timer
		"mrs	x0, 	cntfrq_el0;"	// Get count fequency
		"mov	x1,	2;"
		"mul	x0, 	x0, x1;"
		"msr	cntp_tval_el0, x0;"	// Set expire time
		"mov	x0,	2;"
		"ldr	x1, 	=0x40000040;"	
		"str	w0,	[x1];"		// Unmask the timer interrupt
	);
	uart_puts(" CORE TIMER INITIAL\n");
	return 0;
}

int core_timer_handler(void){
	uart_puts(" Timer handler!\n");
	asm volatile(
		"mrs	x0,	cntfrq_el0;"
		"mov	x1,	2;"
		"mul	x0, 	x0, x1;"
		"msr	cntp_tval_el0,	x0;"
		"eret"
	);
	return 0;
}
