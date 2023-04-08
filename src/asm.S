#define CORE0_TIMER_IRQ_CTRL 0x40000040

.global from_el1_to_el0
from_el1_to_el0:
    //enable irq interrupt bit7 irq interrupt mask set to 0
    mov x2, 0x340
    // TODO WHY 0x3c0
    msr spsr_el1, x2
    // elr_el1 -> program start addr
    msr elr_el1, x0
    // sp_el0 -> usr prog stack addr
    msr sp_el0, x1
    eret

.global core_timer_enable
core_timer_enable:
  mov x0, 1
  msr cntp_ctl_el0, x0 // enable
  // Clock frequency. Indicates the system counter clock frequency, in Hz.
  mrs x0, cntfrq_el0
  msr cntp_tval_el0, x0 // set expired time
  mov x0, 2
  ldr x1, =CORE0_TIMER_IRQ_CTRL
  str w0, [x1] // unmask timer interrupt
  ret

core_timer_handler:
  mrs x0, cntfrq_el0
  msr cntp_tval_el0, x0

.global two_sec_interrupt
two_sec_interrupt:
    mrs x0, cntfrq_el0
    mov x1, 2
    mul x0, x0, x1
    msr cntp_tval_el0, x0
    ret

.global set_timer_expire
set_timer_expire:
    msr cntp_tval_el0, x0
    ret
