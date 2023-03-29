#define CORE0_TIMER_IRQ_CTRL 0x40000040
.global core_timer_enable

core_timer_enable:
    mov x0, 1
    msr cntp_ctl_el0, x0 // enable
    mov x0, 2
    ldr x1, =CORE0_TIMER_IRQ_CTRL
    str w0, [x1] // unmask timer interrupt
    ret

/*
cntpct_el0: The timer’s current count.
cntp_cval_el0: A compared timer count. If cntpct_el0 >= cntp_cval_el0, interrupt the CPU core.
cntp_tval_el0: (cntp_cval_el0 - cntpct_el0). You can use it to set an expired timer after the current timer count.
*/