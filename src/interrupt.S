#include "mmio.h"
#define ARM_PERI_BASE        (KERN_BASE | 0x40000000)
//https://github.com/Tekki/raspberrypi-documentation/blob/master/hardware/raspberrypi/bcm2836/QA7_rev3.4.pdf p7
//https://datasheets.raspberrypi.com/bcm2836/bcm2836-peripherals.pdf 4.6
#define CORE0_TIMER_IRQ_CTRL (ARM_PERI_BASE + 0x40)
//https://datasheets.raspberrypi.com/bcm2836/bcm2836-peripherals.pdf 4.10
#define CORE0_INTERRUPT_SOURCE (ARM_PERI_BASE + 0x60)

.global core_timer_enable
core_timer_enable:
  mov x0, 1
  msr cntp_ctl_el0, x0 // enable
  //https://developer.arm.com/documentation/ddi0601/2020-12/AArch64-Registers/CNTFRQ-EL0--Counter-timer-Frequency-register
  mrs x0, cntfrq_el0 //counter frequency
  msr cntp_tval_el0, x0 // set expired time (one second)
  //https://datasheets.raspberrypi.com/bcm2836/bcm2836-peripherals.pdf
  //4.6
  mov x0, 2
  ldr x1, =CORE0_TIMER_IRQ_CTRL
  str w0, [x1] // unmask timer interrupt
  ret

.global core_timer_disable
core_timer_disable:
  mov x0, 0b0
  msr cntp_ctl_el0, x0 // enable
  //https://developer.arm.com/documentation/ddi0601/2020-12/AArch64-Registers/CNTFRQ-EL0--Counter-timer-Frequency-register
  mov x0, 0
  ldr x1, =CORE0_TIMER_IRQ_CTRL
  str w0, [x1] // unmask timer interrupt
  ret

.global timer_enable_int
timer_enable_int:
    mov x0, 2
    ldr x1, =CORE0_TIMER_IRQ_CTRL
    str w0, [x1] // unmask timer interrupt
    ret

.global timer_disable_int
timer_disable_int:
    mov x0, 0
    ldr x1, =CORE0_TIMER_IRQ_CTRL
    str x0, [x1] // disable timer interrupt
    ret

core_timer_handler:
  mrs x0, cntfrq_el0
  msr cntp_tval_el0, x0
  ret