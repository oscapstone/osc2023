.macro save_el1_context
  sub sp, sp, 2 * 8
  mrs x0, spsr_el1
  mrs x1, elr_el1
  stp x0, x1, [sp, 0]
.endm

.macro load_el1_context
  ldp x0, x1, [sp, 0]
  msr spsr_el1, x0
  msr elr_el1, x1
  add sp, sp, 2 * 8
.endm

// save general registers to stack
.macro save_all
    sub sp, sp, 32 * 8
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
    str x30, [sp, 16 * 15]
.endm

// load general registers from stack
.macro load_all
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
    ldr x30, [sp, 16 * 15]
    add sp, sp, 32 * 8
.endm

.macro exception_entry handler
    b \handler // branch to a handler function.
    .align 7 // entry size is 0x80, .align will pad 0
    
.endm	

exception_handler:
  save_all
  bl default_handler
  load_all
  eret

//align directive: https://developer.arm.com/documentation/dui0473/m/directives-reference/align
.align 11 // vector table should be aligned to 0x800
.global exception_vector_table
//construct table like https://oscapstone.github.io/osc2022/_images/vector_table.jpg
exception_vector_table:
  exception_entry exception_handler
  exception_entry exception_handler
  exception_entry exception_handler
  exception_entry exception_handler

  //Exception from the currentEL while using SP_ELx
  //e.g. kernel takes exceptions(EL1 -> EL1)
  exception_entry _current_synchronuous_exception_handler //Synchronous
  exception_entry _current_irq_exception_handler //IRQ
  exception_entry exception_handler //FIQ
  exception_entry exception_handler //SError

  //Exception from a lower EL and at least one lower EL is AARCH64.
  //e.g. 64bit user programs take exceptions(EL0 -> EL1)
  exception_entry _lower_synchronous_exception_handler //Synchronous
  exception_entry _lower_irq_exception_handler //IRQ
  exception_entry exception_handler //FIQ
  exception_entry exception_handler //SError

  exception_entry exception_handler
  exception_entry exception_handler
  exception_entry exception_handler
  exception_entry exception_handler

.macro handler_template handler_name backbone
\handler_name:
  save_all
  bl \backbone
  load_all
  eret
.endm

.macro el1_handler_template handler_name backbone
\handler_name:
  save_all
  save_el1_context
  bl \backbone
  load_el1_context
  load_all
  eret
.endm

el1_handler_template _current_synchronuous_exception_handler current_synchronous_exception_router
el1_handler_template _current_irq_exception_handler current_irq_exception_router
handler_template _lower_synchronous_exception_handler lower_synchronous_exception_router
handler_template _lower_irq_exception_handler lower_irq_exception_router


.macro globalize symbol
.global \symbol
\symbol:
.endm	

globalize set_exception_vector_table
  adr x0, exception_vector_table
  msr vbar_el1, x0
  ret

globalize disable_local_fiq_interrupt
  msr daifset, #1
  ret
globalize disable_local_irq_interrupt
  msr daifset, #2
  ret
globalize disable_local_async_interrupt
  msr daifset, #4
  ret
globalize disable_local_dbg_interrupt
  msr daifset, #8
  ret
globalize disable_local_all_interrupt
  msr daifset, 0xf
  ret

globalize enable_local_fiq_interrupt
  msr daifclr, #1
  ret
globalize enable_local_irq_interrupt
  msr daifclr, #2
  ret
globalize enable_local_async_interrupt
  msr daifclr, #4
  ret
globalize enable_local_dbg_interrupt
  msr daifclr, #8
  ret
globalize enable_local_all_interrupt
  msr daifclr, 0xf
  ret  
