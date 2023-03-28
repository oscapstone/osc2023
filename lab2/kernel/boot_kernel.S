
.section ".text.boot"

.globl _start
_start:
    mrs    x0, mpidr_el1        
    and    x0, x0, #3        // Check processor id
    cbnz   x0, proc_hang     // Hang for all non-primary CPU

master:
    ldr    x0, =__bss_begin
    ldr    x1, =__bss_end
    sub    x1, x1, x0

memzero:
    cbz    x1, movesp
	str    xzr, [x0], #8
	subs   x1, x1, #8
	cbnz   x1, memzero

movesp:
    mov    sp, #0x400000
    bl     kernel_main

proc_hang: 
    wfe
    b proc_hang