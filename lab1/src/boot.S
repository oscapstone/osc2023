.section ".text.boot"

.global _start

_start:
    // Read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, master     //if CPU id = 0, jump to master
    // Cpu id > 0, stop

halting:  wfe
    b       halting
master:  // Cpu id == 0

    // Set stack before our code
    ldr     x1, =_start
    mov     sp, x1

    // Clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
memzero:  cbz     w2, kernel
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, memzero

    // Jump to C code, should not return
kernel:  bl      main
    // For failsafe, halt this core too
    b       halting