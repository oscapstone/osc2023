#define CORE0_TIMER_IRQ_CTRL 0x40000040

.globl enable_core_timer
enable_core_timer:
        mov     x0, 1
        msr     cntp_ctl_el0, x0 // enable
        mrs     x0, cntfrq_el0
        // TODO: wtf
        ; mov x0, #0xffff
        // Move 16-bit immediate into register, keeping other bits unchanged.
        ; movk x0, #0x7fff, lsl #16
        msr     cntp_tval_el0, x0 // set expired time
        mov     x0, 2
        ldr     x1, =CORE0_TIMER_IRQ_CTRL
        str     w0, [x1] // unmask timer interrupt
        ret

core_timer_handler:
        mrs     x0, cntfrq_el0
        msr     cntp_tval_el0, x0
        ret

.globl reset_core_timer_in_cycle
reset_core_timer_in_cycle:
        msr     cntp_tval_el0, x0
        ret