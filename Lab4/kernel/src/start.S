#include "../include/registers.h"

/* x0 is used for dtb physical address */

.section ".text.boot"
.global _start

_start:



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
    bl          from_el2_to_el1


set_exception_vector_table:
    adr         x1, exception_vector_table // entry.S
    msr         vbar_el1, x1


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
    ldr         x1, =dtb_ptr
    str         x0, [x1], #8
    bl          kernel
    b           proc_hang

memzero:
    str 		xzr, [x0], #8
    subs 		x1, x1, #8
    b.gt 		memzero
    ret

from_el2_to_el1:
    ldr         x1, =CPACR_EL1_VALUE    // Make el0, el1 can use Floating point and Advanced SIMD
    msr         CPACR_EL1, x1
    mov         x1, (1 << 31)           // hcr_el2: Execution state control for EL2
    msr         hcr_el2, x1             //          RW[31]: 0b1 The processor execution environment for EL1 is AArch64
    mov         x1, 0x3c5               // spsr_el2: Holds the saved process state when an exception is taken to EL2.
    msr         spsr_el2, x1            //           hex 0x3c5 = bin 1111000101 (1)[1111] 00 (2)[0101] -> (1) EL2-PSTATE.{D,A,I,F} Disabled (2) Exception level = EL1h
    msr         elr_el2, lr             // elr_el2: When taking an exception to EL2, holds the address to return to.
    eret                                // eret: Perform an exception return. EL2 -> EL1

// reference :
// CPACR_EL1 : https://developer.arm.com/documentation/ddi0601/2020-12/AArch64-Registers/CPACR-EL1--Architectural-Feature-Access-Control-Register
// hcr_el2   : https://developer.arm.com/documentation/ddi0601/2021-12/AArch64-Registers/HCR-EL2--Hypervisor-Configuration-Register
// spsr_el2  : https://developer.arm.com/documentation/ddi0601/2020-12/AArch64-Registers/SPSR-EL2--Saved-Program-Status-Register--EL2-
// elr_el2   : https://developer.arm.com/documentation/ddi0595/2021-06/AArch64-Registers/ELR-EL2--Exception-Link-Register--EL2-
// Note.
// LR : the link register used to hold the return address of a function call.
