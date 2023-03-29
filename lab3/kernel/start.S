#include "mm.h"

/*
 * Everything defined in `boot.S` should go in the `.text.boot` section
 */
.section ".text.boot"

/*
 * Exports the _start symbol for the linker to mark as entry point
 */
.globl _start
_start:
        /*
         * Move device tree address to an empty register
         * TODO: Remove this line when using bootloader
         */
        mov     x28, x0

        bl      from_el2_to_el1
        /*
         * Gets the processor ID from the mpidr_el1 system register
         */
        mrs     x0, mpidr_el1
        and     x0, x0, #0xFF
        cbz     x0, master
        b       proc_hang

from_el2_to_el1:
        /*
         * HCR_EL2, Hypervisor Configuration Register
         * RW   bit[31] 0b1     The Execution state for EL1 is AArch64.
         */
    	mov 	x0, (1 << 31)
    	msr 	hcr_el2, x0
        /*
         * SPSR_EL2, Saved Program Status Register (EL2)
         *
         * D            bit[9]          0b1     Debug exception mask
         * A            bit[8]          0b1     SError interrupt mask
         * I            bit[7]          0b1     IRQ interrupt mask
         * F            bit[6]          0b1     FIQ interrupt mask
         * M[4]         bit[4]          0b0     AArch64 execution state
         * M[3:0]       bit[3:0]        0b0101  EL1h
         *
         * EL1h (SPSel = 1) with interrupt disabled
         * SPSel stands for "stack pointer select"
         */
    	mov 	x0, 0x3c5
    	msr 	spsr_el2, x0
        /*
         * ELR_EL2, Exception Link Register (EL2)
         * LR is link register that hold the return address for a function call.
         */
    	msr	elr_el2, lr
        /*
         * Returns to EL1
         */
        eret

/*
 * Puts all of the other cores in an endless loop
 */
proc_hang: 
        b       proc_hang

/*
 * Works only with the first core
 */
master:
        /*
         * Cleans the .bss section by calling memzero
         * Arguments are passed to the called function via registers x0–x6
         */
        adr     x0, bss_begin
        adr     x1, bss_end
        sub     x1, x1, x0
        bl      memzero

        /*
         * Initializes the stack pointer
         * and pass execution to the kernel_main function
         * LOW_MEMORY is defined in mm.h and is equal to 4MB
         */
        mov     sp, #LOW_MEMORY
        bl      kernel_main