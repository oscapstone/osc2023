#include "../include/registers.h"

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
    cbz         x1, switch
    /* cpu id > 0, stop */

proc_hang:
    /* cpu id > 0 */
    wfe
	b 		    proc_hang 

switch:
    /* cpu id == 0 */
    /* Switch from EL2 to EL1 */
    bl from_el2_to_el1


set_exception_vector_table:
    adr x1, exception_vector_table
    msr vbar_el1, x1


set_stack:
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

from_el2_to_el1:
    ldr     x1, =CPACR_EL1_VALUE     // Make el0, el1 can use Floating point and Advanced SIMD
    msr     CPACR_EL1, x1
    mov x1, (1 << 31) // EL1 uses aarch64
    msr hcr_el2, x1
    mov x1, 0x3c5 // EL1h (SPSel = 1) with interrupt disabled
    msr spsr_el2, x1
    msr elr_el2, lr
    eret // return to EL1