.globl put32
put32:
	str w1,[x0]
	ret

.globl get32
get32:
	ldr w0,[x0]
	ret

.globl delay
delay:
	subs x0, x0, #1
	bne delay
	ret

.globl put64
put64:
	str x1,[x0]
	ret

.globl get64
get64:
	ldr x0,[x0]
	ret
