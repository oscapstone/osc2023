/* See src/lib/exec.c for function declaration */

.global enter_el0_run_user_prog
enter_el0_run_user_prog:
    // set elr_el1 to the program start address
    // Set sp_el0 to the user program's stack pointer
    msr elr_el1, x0
    msr sp_el0, x1
    // set spsr_el1 to 0 to enable interrupt set to 0x3c0 to disable interrupt
    mov x0, 0x0
    msr spsr_el1, x0
    

    // return to EL0
    eret


.global exec_user_prog
exec_user_prog:
    // Set exception return address
    msr elr_el1, x0

    // Set user stack
    msr sp_el0, x1

    // Set kernel stack
    mov sp, x2

    // Enable interrupt, unmask spsr_el1 as 0
    mov x0, 0
    msr spsr_el1, x0

    // return to EL0
    eret