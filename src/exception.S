

.section ".text"

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

.global exception_handler
exception_handler:
    save_all
    bl exception_entry
    load_all
    eret
.global irq_EL1_handler
irq_EL1_handler:
  //ciritcal section
  // msr DAIFSet, 0xf
  save_all
  bl irq_exception_router
  load_all
  // msr DAIFClr, 0xf
  eret

// 0x800 -> 2 ^ 11
// .align 11 means addres would be the multiple of 2 ^ 11
.align 11 // vector table should be aligned to 0x800
.global exception_vector_table
exception_vector_table:
  b exception_handler // branch to a handler function.
  .align 7 // entry size is 0x80, .align will pad 0
  b exception_handler
  .align 7
  b exception_handler
  .align 7
  b exception_handler
  .align 7

  b exception_handler
  .align 7
  b irq_EL1_handler
  .align 7
  b exception_handler
  .align 7
  b exception_handler
  .align 7

  b exception_handler
  .align 7
  b irq_64_lower_el_handler
  .align 7
  b exception_handler
  .align 7
  b exception_handler
  .align 7

  b exception_handler
  .align 7
  b exception_handler
  .align 7
  b exception_handler
  .align 7
  b exception_handler
  .align 7



.global set_exception_vector_table
set_exception_vector_table:
  // adr -> load address to registre 
  adr x0, exception_vector_table
  // Holds the vector base address for any exception that is taken to EL1
  msr vbar_el1, x0
  ret

.global from_el2_to_el1
from_el2_to_el1:
    mov x0, (1 << 31) // EL1 uses aarch64
    /*  
      hcr_el2 -> Provides configuration controls for virtualization, including defining whether various Non-secure operations are trapped to EL2 
      bit[31] -> register width control for lower exception levels
                |  0 -> lower level are all aarch32
                |  1 -> EL1 is aarch64
    */
    msr hcr_el2, x0
    // TODO why set 0x3c5 unknown 0x3c5 -> 0000 
    mov x0, 0x3C5 // EL1h (SPSel = 1) with interrupt disabled
    msr spsr_el2, x0
    msr elr_el2, lr
    eret // return to EL1

irq_64_lower_el_handler:
  save_all
  bl boot_time_2_sec
  load_all
  eret
