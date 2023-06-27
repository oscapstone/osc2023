.section ".text.start"
.extern _stack_top
.extern _bootloader_begin
.extern _kernel_begin
.extern _loader_size
.extern _init
.global _start
_start:
	mov x10, x0
	adrp x0, _stack_top
	mov sp, x0
	adrp x0, _kernel_begin
	adrp x1, _bootloader_begin
	adrp x2, _loader_size
	_loop:
		ldr x4, [x0], #8
		str x4, [x1], #8
		subs x2, x2, #8
		b.gt _loop
	b #(_init - 0x60000)
