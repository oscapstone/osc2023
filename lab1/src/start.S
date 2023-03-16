.section ".text.boot"

.global _start

_start: 
    mrs x0 , mpidr_el1  // CPSR(cpu id) move to x0
    and x0 , x0 ,#0xFF // search core 0
    cbz x0 , bssclear // =0 then jump to main

hold:
    b hold 

    ldr x0 , bss_begin
    ldr x1 , bss_end
    sub x1 , x1,x0 // offset
bssclear:
    cbz x1 , set_stack
    str xzr ,[x0],#8
    sub x1 , x1 ,#8 /***not sure***/
    cbnz x1 , bssclear

set_stack:
    mov sp, #0X80000  // lower than kernel (not sure)

    bl main
