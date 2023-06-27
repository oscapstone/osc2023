.section ".text.start"
.extern _stack_top
.extern _devicetree_begin
.extern _init
.global _start
_start:
	adr x1, _devicetree_begin
	str x0, [x1]
	adrp x0, _stack_top
	//ldr x0, =_stack_top
	mov sp, x0
	bl from_el2_to_el1
	adrp x0, _stack_top
	mov sp, x0
	bl _init
from_el2_to_el1:
    mov x0, (1 << 31) // EL1 uses aarch64
    msr hcr_el2, x0
    mov x0, 0x3c5 // EL1h (SPSel = 1) with interrupt disabled
    msr spsr_el2, x0
    msr elr_el2, lr
    eret // return to EL1
_hang:
	wfe
	b _hang
