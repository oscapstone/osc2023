//other reference: https://willendless.github.io/%E4%BD%93%E7%B3%BB%E7%BB%93%E6%9E%84/2021/03/11/ARM%E6%9E%B6%E6%9E%843/
//http://wiki.csie.ncku.edu.tw/embedded/ARMv8
.global from_el2_to_el1
from_el2_to_el1:
    //https://developer.arm.com/documentation/ddi0601/2021-12/AArch64-Registers/HCR-EL2--Hypervisor-Configuration-Register
    //HCR_EL2 stands for Hypervisor Configuration Register and is a 64-bit register that contains various fields for configuring hypervisor-related features.
    // HCR[31]位默认0表示EL1及以下均为AArch32
    mov x0, (1 << 31) // EL1 uses aarch64
    msr hcr_el2, x0

    //https://developer.arm.com/documentation/dui0473/m/overview-of-the-arm-architecture/saved-program-status-registers
    //Saved Program Status Register, is used to save the PSTATE of the processor when an exception or interrupt occurs in Hypervisor mode, so that it can be restored later when the processor returns from the exception.
    // contains various fields that represent the state of the processor at the time of the exception.
    //set spsr_el2 <- 0x3c5 means
    //NZCV = 0b0000
    //DAIF = 0b1111 (Interrupt Mask Bits)
    //{SS IL} = 0b00
    //EL = 0b01 (Exception Level): the Exception Level at which the exception occurred.
    //nRW = 0b0 //0: 64b, 1: 32b
    //SPSel = 0b1 //0: SP0, 1: SPx
    mov x0, #0x3c5
    msr spsr_el2, x0

    //Exception Link Register  is used to store the address of the instruction that caused an exception or interrupt in Hypervisor mode, so that it can be returned to when the exception is handled.
    msr elr_el2, lr

    //Exception Return
    //In a processor that implements the Virtualization Extensions, you can use ERET to perform a return from an exception taken to Hyp mode.
    eret // return to EL1

//first arg: program address to be run
//second arg: stack address
.global run_user_program
run_user_program:
    //https://developer.arm.com/documentation/ddi0595/2021-06/AArch64-Registers/SPSR-EL1--Saved-Program-Status-Register--EL1-
    //https://willendless.github.io/%E4%BD%93%E7%B3%BB%E7%BB%93%E6%9E%84/2021/03/11/ARM%E6%9E%B6%E6%9E%843/ 1.2.2
    //DAIF = 0b1111, others fields are 0
    //mov x2, #0x3c0
    //allow interrupt
    mov x2, #0x0
    msr spsr_el1, x2
    msr elr_el1, x0 //text address of Program
    msr sp_el0, x1  //stack address of Program
    eret // return to EL0

//https://developer.arm.com/documentation/ddi0595/2021-06/AArch64-Registers/CurrentEL--Current-Exception-Level
.globl get_el
get_el:
   mrs x0, CurrentEl
   lsr x0, x0, #2
   ret

.global update_pgd
update_pgd:
    dsb ish // ensure write has completed
    msr ttbr0_el1, x0 // switch translation based address.
    tlbi vmalle1is // invalidate all TLB entries
    dsb ish // ensure completion of TLB invalidatation
    isb // clear pipeline
    ret