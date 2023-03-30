.section ".text.bootloader"

.global bootloader

bootloader:
    mrs x0, mpidr_el1
    and x0, x0, #0xFF
    cbz x0, set_stack
    b   loop

// hang cpu1 ~ cpu3
loop:
    b   loop

set_stack:
    mov x0, bootloader 

    