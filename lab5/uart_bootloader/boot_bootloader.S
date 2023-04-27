
.section ".text.boot" 

.globl _start
_start:
    mrs    x1, mpidr_el1        
    and    x1, x1, #3           // Check processor id
    cbnz   x1, proc_hang        // Hang for all non-primary CPU
    mov    x20, x0              // move dtb address to x19

master:
    ldr    x0, =__bss_begin     // load address
    ldr    x1, =__bss_end       // load address
    sub    x1, x1, x0           // bss size

memzero:
    cbz    x1, relocate
	str    xzr, [x0], #8
	subs   x1, x1, #8
	cbnz   x1, memzero

relocate:
    ldr    x0, =__begin         // load address
    ldr    x1, =__end           // load address
    sub    x1, x1, x0           // size of all sections
    ldr    x2, =__relocation    // relocation address

relocation_loop:
    ldr    x3, [x0], #8
    str    x3, [x2], #8
    sub    x1, x1, #8
    cbnz   x1, relocation_loop

relocation_done:
    ldr    x0, =bootloader      // load address of bootloader
    ldr    x1, =__begin         // load address
    ldr    x2, =__relocation    // relocation address
    sub    x0, x0, x1
    add    x0, x0, x2
    mov    sp, #0x80000          // move sp to address
    br     x0                   // branch to relocated bootloader

proc_hang: 
    wfe
    b proc_hang