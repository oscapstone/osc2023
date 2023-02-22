.section ".text"

.globl _start
_start:
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, master
    // cpu id > 0, stop

haltloop:  
    wfe
    b       haltloop

master:
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
    b       haltloop