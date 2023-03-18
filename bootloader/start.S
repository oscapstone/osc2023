.section ".text.relo"
.globl _start

_start:
    adr x10, .          //x10=0x80000
    //need to relocate the bootloader from 0x80000 to 0x60000
    ldr x11, =_blsize 
    add x11, x11, x10 
    ldr x12, =_stext    // x12=0x60000

moving_relo:
    cmp x10, x11        //without bootloader
    b.eq end_relo
    ldr x13, [x10]
    str x13, [x12]      //move 0x80000 data to 0x60000
    add x12, x12, #8
    add x10, x10, #8
    b moving_relo
end_relo:
    ldr x14, =_bl_entry    //jump to boot part 
    br x14


.section ".text.boot"
.globl _start_bl
    mrs    x20, mpidr_el1        
    and    x20, x20,#0xFF // Check processor id
    cbz    x20, master   // Hang for all non-primary CPU

hang:
    b hang

master:
    ldr x20, =_dtb 
    str x0, [x20]
    adr    x0, _sbss
    adr    x1, _ebss
    sub    x1, x1, x0
    bl     my_bzero

    mov    sp, #0x400000    // 4MB
    ldr    x1, =_dtb
    ldr    x0, [x1]
    bl    main
    
.global _dtb
.section .data
//The ".dc.a" directive stands for "define constant, absolute address" and is typically used to allocate space for a constant value at a specific memory address.
_dtb: .dc.a 0x0