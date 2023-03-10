.section ".text._reloc"

_reloc:
    // We cannot park the non-boot cores here, since the memory where the park
    // loop resides will be overwritten by the subsequent kernel load.
    // It's fine to let all the cores do the relocation simultaneously, however.

    // Relocate the .text section.

    ldr x0, =_sitext
    ldr x1, =_stext
    ldr x2, =_etext
    b .Lreloc_text_loop_test

.Lreloc_text_loop_body:
    // These instructions do not generate unaligned memory accesses, since the
    // .text section is 16-byte aligned. See `bootloader-reloc.ld`.
    ldp x3, x4, [x0], 16
    stp x3, x4, [x1], 16

.Lreloc_text_loop_test:
    cmp x1, x2
    bne .Lreloc_text_loop_body

    // Relocate the .rodata section.

    ldr x0, =_sirodata
    ldr x1, =_srodata
    ldr x2, =_erodata
    b .Lreloc_rodata_loop_test

.Lreloc_rodata_loop_body:
    // These instructions do not generate unaligned memory accesses, since the
    // .rodata section is 16-byte aligned. See `bootloader-reloc.ld`.
    ldp x3, x4, [x0], 16
    stp x3, x4, [x1], 16

.Lreloc_rodata_loop_test:
    cmp x1, x2
    bne .Lreloc_rodata_loop_body

    // Relocate the .data section.

    ldr x0, =_sidata
    ldr x1, =_sdata
    ldr x2, =_edata
    b .Lreloc_data_loop_test

.Lreloc_data_loop_body:
    // These instructions do not generate unaligned memory accesses, since the
    // .data section is 16-byte aligned. See `bootloader-reloc.ld`.
    ldp x3, x4, [x0], 16
    stp x3, x4, [x1], 16

.Lreloc_data_loop_test:
    cmp x1, x2
    bne .Lreloc_data_loop_body

    // Jump to the relocated entry point to complete initialization.

    b _start

.size _reloc, . - _reloc
.type _reloc, function
.global _reloc
