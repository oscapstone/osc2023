.section ".text.boot"
.globl _start
__relocate_addr = 0x60000     
_start:
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    // # means constant 11(binary) = 3
    and     x1, x1, #3
    // f for froward(down) and b for backward(up)
    cbz     x1, 2f
    // cpu id > 0, stop
1:  
    wfe
    b       1b
2:  // cpu id == 0
    mov x20, x0
    // stack operation is "Full Descending" mode. Need set the top of the stack to the top of the stack space
    ldr x1, =_start
    mov sp, x1
    
    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
3:  cbz     w2, 4f
    // xzr -> 64 bits zero reg , store 0 to x1 address then x1 = x1 + 8 
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b

    // jump to C code, should not return
4:  bl      main
    // for failsafe, halt this core too
    b       1b