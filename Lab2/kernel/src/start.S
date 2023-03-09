/* ARMv8 Assembly Instruction */
/**
mov x0, x1
    sets: x0 = x1
ldr x0, <addr>
    load 32bits from <addr> to x0
ldr w0, <addr>
    load 64bits from <addr> to w0
cbz x0, <label>
    if x0 == 0, jump to <label>
cbnz x0, <label>
    if x0 != 0, jump to <label>
str x0 [x1] #8
    store x0 in addr<x1> then x1=x1+8
b   <label>
    jump to <label>
bl  <label>
    jump to <label> and copies bl's next instruction into link register
wfe
    Wait for event, core in low-power state (power on, clk off)

beq x0, zero, L1
    If x0 = o, jump L1. 

**/

/* x0 is used for dtb physical address */
.section ".text.boot"
.global _start


_start:
    /* store dt loading address to 0x200000 */
    /* ldr     	x1, =0x200000 */
    /* str     	x0, [x1] */            

    b     	    init_bss

init_bss:	
    ldr 		x0, __bss_start
	ldr    		x1, =__stack_top
	mov    		sp, x1
    

    
loop_end:
    bl      kernel
    b       proc_hang



proc_hang:
    wfe
	b 		    proc_hang


