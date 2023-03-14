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
         */
        mov     x28, x0
        /*
         * Gets the processor ID from the mpidr_el1 system register
         */
        mrs     x0, mpidr_el1
        and     x0, x0, #0xFF
        cbz     x0, master
        b       proc_hang

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
         * Sets arguments for relocation loop
         */
        adr     x0, bootloader_start
        adr     x1, relocation_adr
        adr     x2, bootloader_end
        sub     x2, x2, x0

relocation_loop:
        ldr     x3, [x0], #8
        str     x3, [x1], #8
        sub     x2,  x2,  #8
        cbnz    x2, relocation_loop

relocation_branch:
        /*
         * Calculates relocated address of bootloader_main
         */
        adr     x0, bootloader_main
        adr     x1, relocation_adr
        adr     x2, bootloader_start
        sub     x0, x0, x2
        add     x0, x0, x1
        /*
         * Moves stack pointer and branch to relocated address
         */
        mov     sp, #0x80000
        br      x0