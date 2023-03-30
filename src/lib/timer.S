#define CORE0_TIMER_IRQ_CTRL 0x40000040
.global core_timer_enable
core_timer_enable:
    // Enable cntp timer (EL1 physical timer) 
    mov x0, 1
    msr cntp_ctl_el0, x0

    // Set expired time
    mrs x0, cntfrq_el0
    mov x1, #2
    mul x0, x0, x1
    msr cntp_tval_el0, x0

    // Unmask timer interrupt
    mov x0, 2
    ldr x1, =CORE0_TIMER_IRQ_CTRL
    str w0, [x1]

    ret
