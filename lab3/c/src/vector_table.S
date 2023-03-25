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
    vt_entry 0x5

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
