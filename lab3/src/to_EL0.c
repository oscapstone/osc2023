
void exec_in_EL0(char* program_start)
{
	asm volatile
	(
	 	"mov	x1,0x0;"		//open interrupt for 2sec time_interrupt  , 0000: EL0t(jump to EL0)
		"msr	SPSR_EL1,x1;"	//saved process state when an exception is taken to EL1
		"msr	ELR_EL1,x0;"	//put program_start -> ELR_EL1
		"mov	x1,#0x20000;"	//set sp on 0x20000
		"msr	SP_EL0,x1;"		//set EL0 stack pointer
		"ERET"					//exception return , will load from ELR_EL2 -> PC	
	);
	return;
}
