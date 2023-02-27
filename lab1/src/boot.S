#include "mm.h"

// Everything defined in `boot.S` should go in the `.text.boot` section
.section ".text.boot"

// Exports the _start symbol for the linker to mark as entry point
.globl _start
_start:
    // Gets the processor ID from the mpidr_el1 system register    
    mrs    x0, mpidr_el1
    and    x0, x0, #0xFF
    cbz    x0, master
    b      proc_hang

// Puts all of the other cores in an endless loop
proc_hang: 
    b      proc_hang

// Works only with the first core
master:
    adr    x0, bss_begin
    adr    x1, bss_end
    sub    x1, x1, x0
    // Cleans the .bss section by calling memzero
    // Arguments are passed to the called function via registers x0–x6
    bl     memzero

    // Initializes the stack pointer
    // and pass execution to the kernel_main function
    // LOW_MEMORY is defined in mm.h and is equal to 4MB
    mov    sp, #LOW_MEMORY
    bl     kernel_main