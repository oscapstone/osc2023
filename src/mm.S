.globl my_bzero
my_bzero:
	str xzr, [x0], #8 // set the byte x0 point to zero
	subs x1, x1, #8   // size--
	b.gt my_bzero
	ret