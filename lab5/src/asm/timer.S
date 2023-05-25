#define CORE0_TIMER_IRQ_CTRL 0x40000040

.global sys_enable_core_timer
sys_enable_core_timer:
    mov x0, 1
    msr cntp_ctl_el0, x0            // enable

    mrs x0, cntfrq_el0
    mov x1, 5
    // mul x0, x0, x1
    lsr x0, x0, x1
    msr cntp_tval_el0, x0           // set expired time

    mrs x0, cntkctl_el1
    mov x1, 1
    orr x0, x0, x1
    msr cntkctl_el1, x0
    
    mov x0, 2
    ldr x1, =CORE0_TIMER_IRQ_CTRL
    str w0, [x1]                    // unmask timer 
    ret

.global sys_disable_core_timer
sys_disable_core_timer:
    mrs x0, cntp_ctl_el0
    mov x1, 1
    eor x1, x1, x1                  // not x1
    and x0, x0, x1
    msr cntp_ctl_el0, x0

    ldr x2, =CORE0_TIMER_IRQ_CTRL
    ldr x0, [x2]
    mov x1, 2
    eor x1, x1, x1                  // not x1
    and x0, x0, x1
    str x0, [x2]                    // enable timer interrupt
    ret

    