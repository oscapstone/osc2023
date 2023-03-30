.section ".text.boot"

.global _start
_start:
		mov 	x27,x0			//save for dtb use

		ldr		x0,=bss_begin	//clean bss first
		ldr		x1,=bss_size
memzero:
		cbz		x1,set_loc		//if bss_size = 0 , branch to set_loc
		str		xzr,[x0],#8		//write zero register(xzr)'s value to x0's addr , and x0=x0+8
		sub		x1,x1,#1
		cbnz	x1,memzero		//if bss_size != 0 , branch memzero(repeat clean)

set_loc:
		ldr		x2,=loader_size
		mov		x0,#0x80000
		mov		x1,#0x60000
copy:
		ldr		x3,[x0],#8		//load x0(loader img) -> x3 , and x0=x0+8
		str		x3,[x1],#8		//store x3 -> x1(0x60000) , and x1=x1+8
		sub		x2,x2,#1		//loader_size-=1
		cbnz	x2,copy			//repeat copy

		mov		sp,#0x60000		//set stack point
master:
		bl		jp2loader
		mov		x0,#0x80000
		str		xzr,[x0]
		bl 		kernel_main
		b		copy
jp2loader:
		sub		x30,x30,#(0x80000-0x60000)
		ret
