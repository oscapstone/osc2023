#include <loader.h>

/**************************************************************************
 * This function will run the program at the specific location.
 * And this function sould handle following tasks.
 * 1. Save kernel status.
 * 2. EL1 -> EL0.
 * 3. Start user program.
 * ***********************************************************************/
int run_program(void* loc){
	asm volatile(
		"mov x1,	0x0;\r\n"	// Enable CPU interrupt
		"msr spsr_el1, 	x1;\r\n"
		"mov x1,	0x40000;\r\n"	// Set user stack to 0x60000
		"msr sp_el0,	x1;\r\n"
		"mov x1,	%[loc];\r\n"	
		"msr elr_el1,	x1;\r\n"	// Set the target address
		"mov x0,	%[loc];\r\n"	// For recalculat offset
		"eret"
		:
		: [loc] "r" (loc)	// input location
	);

	// Note: at this stage, this function will not return.
	return 0;
}
