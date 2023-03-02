.section ".text.boot"
.globl _start
_start:
    mrs x0, mpidr_el1
    and x0, x0, #0xFF
    cbz x0, master
    b proc_hang

.globl memzero
memzero:
    str xzr, [x0], #8
    subs x1, x1, #8
    b.gt memzero
    ret

proc_hang:
    b proc_hang


master:
    ldr x1, =_stack_begin
    mov sp, x1

    adr x0, _bss_begin
    adr x1, _bss_end
    sub x1, x1, x0
    bl memzero

    bl kernel_main
    b proc_hang
