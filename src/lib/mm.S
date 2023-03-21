.global memzero
memzero:
    str xzr, [x0], #8
    subs x1, x1, #8
    b.gt memzero
    ret

// dest x0, src x1, len x2
.globl memncpy
memncpy:
    ldrb w3, [x1], #1
    strb w3, [x0], #1
    subs x2, x2, #1
    b.gt memncpy
    ret