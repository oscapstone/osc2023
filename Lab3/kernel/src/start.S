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
    /* if the kernel is loaded directly */
    /* ldr     	x1, =0x200000 */
    /* str     	x0, [x1] */   
    /* read cpu id, stop slave cores */
    mrs         x1, mpidr_el1
    and         x1, x1, #3
    cbz         x1, set_stack 
    /* cpu id > 0, stop */

proc_hang:
    /* cpu id > 0 */
    wfe
	b 		    proc_hang 

set_stack:
    /* cpu id == 0 */
    /* set stack before our code */
    ldr         x1, =_start
    mov         sp, x1


    /* clear bss */
    ldr         x1, =__bss_start
    ldr         w2, =__bss_size
init_bss:	
    cbz         w2, loop_end
    str         xzr, [x1], #8
    sub         w2, w2, #1
    cbnz        w2, init_bss
    
loop_end:
    bl          kernel
    b           proc_hang

memzero:
    str 		xzr, [x0], #8
    subs 		x1, x1, #8
    b.gt 		memzero
    ret