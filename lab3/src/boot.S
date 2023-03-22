.section ".text.boot"

.global _start
_start:
		mov 	x27,x0;
		mrs		x0,mpidr_el1	//mrs: move processorID to ARM register from mpidr_el1 register
		and		x0,x0,#0x3		//check processor ID
		cbz		x0,cleanbss		//if processor ID = 0(we want only one core) , branch to cleanbss

proc_hang:
		b		proc_hang

cleanbss:
		ldr		x0,=_start
		mov		sp,x0			//set stack start address
		ldr		x0,=bss_begin
		ldr		x1,=bss_size
memzero:
		cbz		x1,master		//if bss_size = 0 , branch to master
		str		xzr,[x0],#8		//write zero register(xzr)'s value to x0's addr , and x0=x0+8
		sub		x1,x1,#1
		cbnz	x1,memzero		//if bss_size != 0 , branch memzero(repeat clean)

setup_EL1:
		bl		from_EL2_to_EL1

El1_start:
		mov		sp,#0x70000		//put EL1 stack start from 0x70000	

master:
		mov		x0,x27
		bl		kernel_main
		b 		proc_hang

from_EL2_to_EL1:
		mov		x0,(1<<31)		//in HCR_EL2 , 31bit set : the execution state for EL1 is AArch64
		msr		HCR_EL2,x0		//Hypervisor Configuration Register for EL2 , msr:load general register -> state register
		mov		x0,0x3C5		//0011:	9:PSTATE.D 8:PSTATE.A , 1100: 7:PSTATE.I 6:PSTATE.F(D,A,I,F set 1 to disable interrupt) , 0101: EL1h(jump to EL1)
		msr		SPSR_EL2,x0		//Saved Program Status Register (EL2) , saved process state when an exception is taken to EL2
		msr		ELR_EL2,LR		//Exception Link Register (EL2) , when taking an exception to EL2, holds the address to return to
		ERET					//exception return , will load from ELR_EL2 -> PC
