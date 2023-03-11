.globl memzero
memzero:
	str    	xzr, [x0], #8
	/* Decreases by 8 and compare to zero */
	subs   	x1, x1, #8
	b.gt	memzero
	ret