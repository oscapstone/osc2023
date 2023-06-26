#include "mmu/mmu-def.h"

.section ".text.mmu"
.global enable_kernel_mmu
enable_kernel_mmu:
    // setup kernel virtaul memory
    mov x4, x0
    ldr x0, =TCR_CONFIG_DEFAULT
    msr tcr_el1, x0

    ldr x0, =(\
        (MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | \
        (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)) \
    )
    msr mair_el1, x0

    mov x0, 0x1000 // kernel PGD
    mov x1, 0x2000 // kernel PUD
    mov x5, 0x3000 // kernel PMD

    // set PGD entry
    ldr x2, =BOOT_PGD_ATTR
    orr x2, x1, x2
    str x2, [x0]


    ldr x2, =BOOT_PUD_TABLE
    orr x3, x5, x2
    str x3, [x1]
    ldr x2, =BOOT_PMD_ATTR_NORMAL

    mov x6, 0
    mov x7, 0
    .setup_normal_kernel_PMD:
        orr x3, x6, x2
        str x3, [x5], 8
        add x6, x6, 0x200000
        add x7, x7, 1
        cmp x7, #480
        blt .setup_normal_kernel_PMD

    ldr x2, =BOOT_PMD_ATTR_DEVICE

    .setup_device_kernel_PMD:
        orr x3, x6, x2
        str x3, [x5], 8
        add x6, x6, 0x200000
        add x7, x7, 1
        cmp x7, 512
        blt .setup_device_kernel_PMD

    // ldr x2, =BOOT_PUD_ATTR_NORMAL
    // mov x3, 0x00000000
    // orr x3, x2, x3
    // str x3, [x1]

    ldr x2, =BOOT_PUD_ATTR_DEVICE
    mov x3, 0x40000000
    orr x3, x2, x3
    str x3, [x1, 8]

    msr ttbr0_el1, x0
    msr ttbr1_el1, x0

    mrs x2, sctlr_el1
    orr x2, x2, 1
    msr sctlr_el1, x2
    br x4