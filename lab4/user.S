.section ".text"
.global _start
_start:
	mov	x0, 0
1:
	add	x0, x0, 1
	svc 	0		// Exception
	cmp	x0, 5
	blt	1b
2:
	b 	2b		// Stuck
