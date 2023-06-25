.macro save_aapcs
    stp x0, x1, [sp, -(20 * 8)]!
    stp x2, x3, [sp, 2 * 8]
    stp x4, x5, [sp, 4 * 8]
    stp x6, x7, [sp, 6 * 8]
    stp x8, x9, [sp, 8 * 8]
    stp x10, x11, [sp, 10 * 8]
    stp x12, x13, [sp, 12 * 8]
    stp x14, x15, [sp, 14 * 8]
    stp x16, x17, [sp, 16 * 8]
    stp x18, x30, [sp, 18 * 8]
.endm

.macro load_aapcs
    ldp x18, x30, [sp, 18 * 8]
    ldp x16, x17, [sp, 16 * 8]
    ldp x14, x15, [sp, 14 * 8]
    ldp x12, x13, [sp, 12 * 8]
    ldp x10, x11, [sp, 10 * 8]
    ldp x8, x9, [sp, 8 * 8]
    ldp x6, x7, [sp, 6 * 8]
    ldp x4, x5, [sp, 4 * 8]
    ldp x2, x3, [sp, 2 * 8]
    ldp x0, x1, [sp], (20 * 8)
.endm
