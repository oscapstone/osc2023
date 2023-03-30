#include "oscos/save_all.S"

.macro vt_entry vten
save_aapcs
mov x0, \vten
bl exception_handler
load_aapcs
eret
.align 7
.endm

.section ".text"

.align 11
vector_table:
    // Exception from the current EL while using SP_EL0.

    // Synchronous.
    vt_entry 0x0

    // IRQ.
    vt_entry 0x1

    // FIQ.
    vt_entry 0x2

    // SError.
    vt_entry 0x3

    // Exception from the current EL while using SP_ELx.

    // Synchronous.
    vt_entry 0x4

    // IRQ.
    save_aapcs
    bl irq_handler_el1
    stp x19, x20, [sp, -(14 * 8)]!
    stp x21, x22, [sp, 2 * 8]
    stp x23, x24, [sp, 4 * 8]
    stp x25, x26, [sp, 6 * 8]
    stp x27, x28, [sp, 8 * 8]
    add x0, sp, 34 * 8
    stp x29, x0, [sp, 10 * 8]
    mrs x0, spsr_el1
    mrs x1, elr_el1
    stp x0, x1, [sp, 12 * 8]
    mov x0, sp
    b task_queue_entry
    .align 7

    // FIQ.
    vt_entry 0x6

    // SError.
    vt_entry 0x7

    // Exception from a lower EL and at least one lower EL is AArch64.

    // Synchronous.
    vt_entry 0x8

    // IRQ.
    save_aapcs
    bl irq_handler
    load_aapcs
    eret
    .align 7

    // FIQ.
    vt_entry 0xa

    // SError.
    vt_entry 0xb

    // Exception from a lower EL and all lower ELs are AArch32.

    // Synchronous.
    vt_entry 0xc

    // IRQ.
    vt_entry 0xd

    // FIQ.
    vt_entry 0xe

    // SError.
    vt_entry 0xf

.size vector_table, . - vector_table
.global vector_table
