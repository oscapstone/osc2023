#include "oscos/utils/save-ctx.S"

.macro default_vt_entry vten label_name
\label_name:
    save_aapcs
    mov x0, \vten
    bl xcpt_default_handler
    load_aapcs
    eret

.type \label_name, function
.size \label_name, . - \label_name

.align 7
.endm

.section ".text"

.align 11
xcpt_vector_table:
    // Exception from the current EL while using SP_EL0.

    // Synchronous.
    default_vt_entry 0x0 xcpt_sync_curr_el_sp_el0_handler

    // IRQ.
    default_vt_entry 0x1 xcpt_irq_curr_el_sp_el0_handler

    // FIQ.
    default_vt_entry 0x2 xcpt_fiq_curr_el_sp_el0_handler

    // SError.
    default_vt_entry 0x3 xcpt_serror_curr_el_sp_el0_handler

    // Exception from the current EL while using SP_ELx.

    // Synchronous.
    default_vt_entry 0x4 xcpt_sync_curr_el_sp_elx_handler

    // IRQ.
xcpt_irq_curr_el_sp_elx_handler:
    save_aapcs
    bl xcpt_irq_handler
    bl task_queue_sched
    load_aapcs
    eret

.type xcpt_irq_curr_el_sp_elx_handler, function
.size xcpt_irq_curr_el_sp_elx_handler, . - xcpt_irq_curr_el_sp_elx_handler

.align 7

    // FIQ.
    default_vt_entry 0x6 xcpt_fiq_curr_el_sp_elx_handler

    // SError.
    default_vt_entry 0x7 xcpt_serror_curr_el_sp_elx_handler

    // Exception from a lower EL and at least one lower EL is AArch64.

    // Synchronous.
    default_vt_entry 0x8 xcpt_sync_lower_el_aarch64_handler

    // IRQ.
xcpt_irq_lower_el_aarch64_handler:
    save_aapcs
    bl xcpt_irq_handler
    bl task_queue_sched
    load_aapcs
    eret

.type xcpt_irq_lower_el_aarch64_handler, function
.size xcpt_irq_lower_el_aarch64_handler, . - xcpt_irq_lower_el_aarch64_handler

.align 7

    // FIQ.
    default_vt_entry 0xa xcpt_fiq_lower_el_aarch64_handler

    // SError.
    default_vt_entry 0xb xcpt_serror_lower_el_aarch64_handler

    // Exception from a lower EL and all lower ELs are AArch32.

    // Synchronous.
    default_vt_entry 0xc xcpt_sync_lower_el_aarch32_handler

    // IRQ.
    default_vt_entry 0xd xcpt_irq_lower_el_aarch32_handler

    // FIQ.
    default_vt_entry 0xe xcpt_fiq_lower_el_aarch32_handler

    // SError.
    default_vt_entry 0xf xcpt_serror_lower_el_aarch32_handler

.size xcpt_vector_table, . - xcpt_vector_table
.global xcpt_vector_table
