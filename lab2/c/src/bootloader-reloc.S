.section ".text._reloc"

_reloc:
    // We cannot park the non-boot cores here, since the memory where the park
    // loop resides will be overwritten by the subsequent kernel load.
    // It's fine to let all the cores do the relocation simultaneously, however.

    // Relocate the .text section.

    ldr x1, =_sitext
    ldr x2, =_stext
    ldr x3, =_etext
    b .Lreloc_text_loop_test

.Lreloc_text_loop_body:
    // These instructions do not generate unaligned memory accesses, since the
    // .text section is 16-byte aligned. See `bootloader-reloc.ld`.
    ldp x4, x5, [x1], 16
    stp x4, x5, [x2], 16

.Lreloc_text_loop_test:
    cmp x2, x3
    bne .Lreloc_text_loop_body

    // Relocate the .rodata section.

    ldr x1, =_sirodata
    ldr x2, =_srodata
    ldr x3, =_erodata
    b .Lreloc_rodata_loop_test

.Lreloc_rodata_loop_body:
    // These instructions do not generate unaligned memory accesses, since the
    // .rodata section is 16-byte aligned. See `bootloader-reloc.ld`.
    ldp x4, x5, [x1], 16
    stp x4, x5, [x2], 16

.Lreloc_rodata_loop_test:
    cmp x2, x3
    bne .Lreloc_rodata_loop_body

    // Relocate the .data section.

    ldr x1, =_sidata
    ldr x2, =_sdata
    ldr x3, =_edata
    b .Lreloc_data_loop_test

.Lreloc_data_loop_body:
    // These instructions do not generate unaligned memory accesses, since the
    // .data section is 16-byte aligned. See `bootloader-reloc.ld`.
    ldp x4, x5, [x1], 16
    stp x4, x5, [x2], 16

.Lreloc_data_loop_test:
    cmp x2, x3
    bne .Lreloc_data_loop_body

    // Jump to the relocated entry point to complete initialization.

    b _start

.size _reloc, . - _reloc
.type _reloc, function
.global _reloc
