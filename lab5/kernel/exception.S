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

.globl branch_to_address_el0
branch_to_address_el0:
        /*
         * SPSR_EL1, Saved Program Status Register (EL1)
         *
         * D            bit[9]          0b1     Debug exception mask
         * A            bit[8]          0b1     SError interrupt mask
         * I            bit[7]          0b0     IRQ interrupt mask
         * F            bit[6]          0b1     FIQ interrupt mask
         * M[4]         bit[4]          0b0     AArch64 execution state
         * M[3:0]       bit[3:0]        0b0000  EL0t
         */
        mov     x2, 0x340
        msr     spsr_el1, x2
        /*
         * address of user program
         */
        msr     elr_el1, x0
        /*
         * sp of user program
         */
        msr     sp_el0, x1
        eret

exception_handler_system_call:
        save_all
        bl print_exception_info
        load_all
        eret

exception_handler_irq_64_el0:
        save_all
        bl irq_64_el0
        load_all
        eret

exception_handler_irq_64_el1:
        save_all
        bl irq_64_el1
        load_all
        eret

exception_handler_nothing:
        save_all
        // do nothing
        load_all
        eret

.align 11 // vector table should be aligned to 0x800
.global exception_vector_table
exception_vector_table:
        /*
         * el not change, using sp_el0
         */
        b       exception_handler_nothing       // synchronos exception
        .align 7
        b       exception_handler_nothing       // IRQ and vIRQ exception
        .align 7
        b       exception_handler_nothing       // FIQ and vFIQ exception
        .align 7
        b       exception_handler_nothing       // SError and vSError
        .align 7

        /*
         * el not change, using sp_elx
         */
        b       exception_handler_nothing       // synchronos exception
        .align 7
        b       exception_handler_irq_64_el1    // IRQ and vIRQ exception
        .align 7
        b       exception_handler_nothing       // FIQ and vFIQ exception
        .align 7
        b       exception_handler_nothing       // SError and vSError
        .align 7

        /*
         * Moves to a higher el, from aarch64
         */
        b       exception_handler_system_call   // synchronos exception
        .align 7
        b       exception_handler_irq_64_el0    // IRQ and vIRQ exception
        .align 7
        b       exception_handler_nothing       // FIQ and vFIQ exception
        .align 7
        b       exception_handler_nothing       // SError and vSError
        .align 7

        /*
         * Moves to a higher el, from aarch32
         */
        b       exception_handler_nothing       // synchronos exception
        .align 7
        b       exception_handler_nothing       // IRQ and vIRQ exception
        .align 7
        b       exception_handler_nothing       // FIQ and vFIQ exception
        .align 7
        b       exception_handler_nothing       // SError and vSError
        .align 7

.global set_exception_vector_table
set_exception_vector_table:
        adr     x0, exception_vector_table
        msr     vbar_el1, x0
        ret