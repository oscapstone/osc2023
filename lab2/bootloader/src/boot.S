.section ".text.boot"

.global _start

_start:
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, master
    // cpu id > 0, stop
proc_hang: // hang rest cpu
    wfe
    b       proc_hang
master:  // cpu id == 0

    // set top of stack just before our code (stack grows to a lower address per AAPCS64)
    ldr     x1, =__stack_start
    mov     sp, x1

    // set bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
memzero:  // initial bss then 4f
    cbz     w2, entry
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, memzero
    
entry:  // jump to C code, should not return
    // normal set dtb to x0
    ldr     x1, =_dtb
    str     x0, [x1], #8
    
    bl      main
    // for failsafe, halt this core too
    b       proc_hang