
void exec_in_EL0(char* program_start,char* stack)
{
	asm volatile
	(
	 	"mov	x2,0x0;"		//0000: EL0t(jump to EL0)
		"msr	SPSR_EL1,x2;"	//saved process state when an exception is taken to EL1
		"msr	ELR_EL1,x0;"	//put program_start -> ELR_EL1
		"msr	SP_EL0,x1;"		//set EL0 stack pointer
		"ERET"					//exception return , will load from ELR_EL2 -> PC	
	);
	return;
}
