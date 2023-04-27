.section ".text"
.global start
start:
    mrs x0, cntfrq_el0
    add x0, x0, x0
    msr cntp_tval_el0, x0 // set expired time
1:
    b 1b