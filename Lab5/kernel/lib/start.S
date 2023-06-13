.section ".text.boot"

.global _start

_start:
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, master
    // cpu id > 0, stop

halt:
    wfe
    b       halt

from_el2_to_el1:
    mov x1, (1 << 31) // EL1 uses aarch64
    msr hcr_el2, x1
    mov x1, 0x3c5     // Set jump target EL1h and EL1h interrupt disabled
    msr spsr_el2, x1
    msr elr_el2, lr   // Set position to go to in EL1h
    eret              // return to EL1

master:
    bl      from_el2_to_el1

    mov     x1, (3 << 20)
    msr     CPACR_EL1, x1 // for printf: float, SIMD

    adr     x1, exception_vector_table // set exception_vector_table
    msr     vbar_el1, x1
    
    ldr     x1, =_start
    mov     sp, x1

    ldr     x1, =__bss_start
    ldr     w2, =__bss_size

bssinitloop:
    // use a loop to initialize every position of bss to zero
    cbz     w2, kernel
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, bssinitloop

kernel:
    // jump to C code
    bl      main
    // if fail
    b       halt
