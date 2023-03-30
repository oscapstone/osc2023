#define CORE0_TIMER_IRQ_CTRL 0x40000040
#define CORE1_TIMER_IRQ_CTRL 0x40000044
#define CORE2_TIMER_IRQ_CTRL 0x40000048
#define CORE3_TIMER_IRQ_CTRL 0x4000004C

.extern exception_entry
.extern spx_irq_handler
.extern lower_irq_handler
.extern enable_interrupt
.extern disable_interrupt
.extern event_queue_handle

.section ".text.boot"
.globl _start
_start:
    mov x10, x0
    mov x11, x1
    mov x12, x2
    mov x13, x3
    bl from_el2_to_el1
    mrs x0, mpidr_el1
    and x0, x0, #0xFF
    cbz x0, master
    b proc_hang

// switch to el1
from_el2_to_el1:
    mov x0, (1 << 31)
    // make execution state for el1 is aarch64
    msr hcr_el2, x0
    mov x0, 0x3c5
    msr spsr_el2, x0
    msr elr_el2, lr
    eret

__memzero:
    str xzr, [x0], #8
    subs x1, x1, #8
    b.gt __memzero
    ret

proc_hang:
    b proc_hang


master:
// set stack pointer
    ldr x1, =_stack_begin
    mov sp, x1

// clear bss section
    adr x0, _bss_begin
    adr x1, _bss_end
    sub x1, x1, x0
    bl __memzero

// set exception_vector table
    bl set_exception_vector_table
    bl core_timer_enable
    mov x0, x10
    mov x1, x11
    mov x2, x12
    mov x3, x13

    msr DAIFClr, 0xf
    bl kernel_main
    b proc_hang


.macro save_all
    sub sp, sp, 32 * 9
    stp x0, x1, [sp ,16 * 0]
    stp x2, x3, [sp ,16 * 1]
    stp x4, x5, [sp ,16 * 2]
    stp x6, x7, [sp ,16 * 3]
    stp x8, x9, [sp ,16 * 4]
    stp x10, x11, [sp ,16 * 5]
    stp x12, x13, [sp ,16 * 6]
    stp x14, x15, [sp ,16 * 7]
    stp x16, x17, [sp ,16 * 8]
    stp x18, x19, [sp ,16 * 9]
    stp x20, x21, [sp ,16 * 10]
    stp x22, x23, [sp ,16 * 11]
    stp x24, x25, [sp ,16 * 12]
    stp x26, x27, [sp ,16 * 13]
    stp x28, x29, [sp ,16 * 14]
    mrs x0, elr_el1
    stp x30, x0, [sp, 16 * 15]
    mrs x0, spsr_el1
    str x0, [sp, 16 * 16]
.endm

// load general registers from stack
.macro load_all
    ldr x0, [sp, 16 * 16]
    msr spsr_el1, x0
    ldp x30, x0, [sp, 16 * 15]
    msr elr_el1, x0
    ldp x0, x1, [sp ,16 * 0]
    ldp x2, x3, [sp ,16 * 1]
    ldp x4, x5, [sp ,16 * 2]
    ldp x6, x7, [sp ,16 * 3]
    ldp x8, x9, [sp ,16 * 4]
    ldp x10, x11, [sp ,16 * 5]
    ldp x12, x13, [sp ,16 * 6]
    ldp x14, x15, [sp ,16 * 7]
    ldp x16, x17, [sp ,16 * 8]
    ldp x18, x19, [sp ,16 * 9]
    ldp x20, x21, [sp ,16 * 10]
    ldp x22, x23, [sp ,16 * 11]
    ldp x24, x25, [sp ,16 * 12]
    ldp x26, x27, [sp ,16 * 13]
    ldp x28, x29, [sp ,16 * 14]
    // ldr x30, [sp, 16 * 15]
    add sp, sp, 32 * 9
.endm

exception_handler:
    save_all
    bl disable_interrupt
    mrs x0, esr_el1
    mrs x1, elr_el1
    mrs x2, spsr_el1
    bl exception_entry
    bl enable_interrupt
    load_all
    eret

.balign 0x800
.global exception_vector_table
exception_vector_table:
    b exception_handler
    .balign 0x80
    b exception_handler
    .balign 0x80
    b exception_handler
    .balign 0x80
    b exception_handler
    .balign 0x80
    b exception_handler
    .balign 0x80
    b el_aarch64_irq_epx
    .balign 0x80
    b exception_handler
    .balign 0x80
    b exception_handler
    .balign 0x80
    b exception_handler
    .balign 0x80
    b _lower_el_aarch64_irq
    .balign 0x80
    b exception_handler
    .balign 0x80
    b exception_handler
    .balign 0x80
    b exception_handler
    .balign 0x80
    b exception_handler
    .balign 0x80
    b exception_handler
    .balign 0x80
    b exception_handler
    .balign 0x80

set_exception_vector_table:
    adr x0, exception_vector_table
    msr vbar_el1, x0
    ret

core_timer_enable:
// let el0 can use cntp_tval_el0 and cntfrq_el0 without causing interrupt
    mrs x0, cntkctl_el1
    orr x0, x0, #1
    orr x0, x0, #8
    orr x0, x0, #256
    orr x0, x0,#512
    msr cntkctl_el1, x0
    mov x0, 1
    msr cntp_ctl_el0, x0 // enable
    mrs x0, cntfrq_el0
    // set some expired time
    ldr x0, =0xffffffffff
    msr cntp_cval_el0, x0// set expired time
    mov x0, 2
    ldr x1, =CORE0_TIMER_IRQ_CTRL
    str w0, [x1] // unmask timer interrupt
    ret

.global core_timer_handler
core_timer_handler:
    mrs x0, cntfrq_el0
    msr cntp_tval_el0, x0
    ret
el_aarch64_irq_epx:
    msr DAIFSet, 0xf
    save_all
    mrs x0, esr_el1
    mrs x1, elr_el1
    mrs x2, spsr_el1
    bl spx_irq_handler
    msr DAIFClr, 0xf
    bl event_queue_handle

    msr DAIFSet, 0xf
    load_all
    msr DAIFClr, 0xf
    eret
_lower_el_aarch64_irq:
    msr DAIFSet, 0xf
    save_all
    mrs x0, esr_el1
    mrs x1, elr_el1
    mrs x2, spsr_el1
    bl lower_irq_handler
    msr DAIFClr, 0xf
    
    bl event_queue_handle
    msr DAIFSet, 0xf
    load_all
    msr DAIFClr, 0xf
    eret