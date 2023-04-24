.global get_current_process
get_current_process:
        mrs     x0, tpidr_el0
        ret

.global set_current_process
set_current_process:
        msr     tpidr_el0, x0
        ret