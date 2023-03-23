.section ".text._start"

_start:
    // Clear the .bss section.

    ldr x1, =_sbss
    ldr x2, =_ebss
    b .Lclear_bss_loop_test

.Lclear_bss_loop_body:
    // This does not generate unaligned memory accesses, since the .bss section
    // is 16-byte aligned. See `linker.ld`.
    stp xzr, xzr, [x1], 16

.Lclear_bss_loop_test:
    cmp x1, x2
    b.ne .Lclear_bss_loop_body

    // Set the stack pointer.

    ldr x1, =_estack
    mov sp, x1

    // Call the main function.

    bl main

    // Park the core.

.Lpark_loop:
    wfe
    b .Lpark_loop

.size _start, . - _start
.type _start, function
.global _start
