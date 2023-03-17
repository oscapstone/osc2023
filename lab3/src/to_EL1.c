
void exec_in_EL0(char* program_start)
{
	
	asm volatile
	(
		"mov	x0,0x3C0;"		//0011: 9:PSTATE.D 8:PSTATE.A , 1100: 7:PSTATE.I 6:PSTATE.F(D,A,I,F set 1 to disable interrupt) , 0000: EL0t(jump to EL0)
		"msr	SPSR_EL1,x0;"	//saved process state when an exception is taken to EL1
		"msr	ELR_EL1,;"
		"mov	SP_EL0,#0x40000;"
		"ERET"					//exception return , will load from ELR_EL2 -> PC	
	);
}
