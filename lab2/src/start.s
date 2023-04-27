.section ".text.start"
.extern _stack_top
.extern _init
.global _start
_start:
	adrp x0, _stack_top
	//ldr x0, =_stack_top
	mov sp, x0
	bl _init
_hang:
	wfe
	b _hang
