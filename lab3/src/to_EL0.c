#include "to_EL0.h"

void exec_in_EL0(char* program_start)
{
	asm volatile
	(
	 	"mov	x1,0x3C0;"		//0011: 9:PSTATE.D 8:PSTATE.A , 1100: 7:PSTATE.I 6:PSTATE.F(D,A,I,F set 1 to disable interrupt) , 0000: EL0t(jump to EL0)
		"msr	SPSR_EL1,x1;"	//saved process state when an exception is taken to EL1
		"msr	ELR_EL1,x0;"	//put program_start -> ELR_EL1
		"mov	x1,#0x40000;"	//set sp on 0x40000
		"msr	SP_EL0,x1;"		//set EL0 stack pointer
		"ERET"					//exception return , will load from ELR_EL2 -> PC	
	);
	return;
}
