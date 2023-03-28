.global exec_user_proc
exec_user_proc:
    // Set sp_el0 to the user program's stack pointer
    msr sp_el0, x0
    
    // set spsr_el1 to 0 to enable interrupt set to 0x3c0 to disable interrupt
    mov x0, 0x0
    msr spsr_el1, x0
    // set elr_el1 to the program start address
    msr elr_el1, x1

    // return to EL0
    eret