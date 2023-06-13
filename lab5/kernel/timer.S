#define CORE0_TIMER_IRQ_CTRL 0x40000040

.globl enable_core_timer
enable_core_timer:
        /*
         * enables timer interrupt
         */
        mov     x0, 1
        msr     cntp_ctl_el0, x0
        /*
         * sets expired time to 1 second
         */
        mrs     x0, cntfrq_el0
        msr     cntp_tval_el0, x0
        /*
         * unmasks timer interrupt
         */
        mov     x0, 2
        ldr     x1, =CORE0_TIMER_IRQ_CTRL
        str     w0, [x1]
        /*
         * allows cpu timer register access
         */
        mrs     x0, cntkctl_el1
        orr     x0, x0, #1
        msr     cntkctl_el1, x0
        ret

.globl reset_core_timer_in_cycle
reset_core_timer_in_cycle:
        msr     cntp_tval_el0, x0
        ret

.globl reset_core_timer_absolute
reset_core_timer_absolute:
        msr     cntp_cval_el0, x0
        ret