.globl memzero
memzero:
    str     xzr, [x0], #8   //xzr:zero register
    subs    x1, x1, #8
    b.gt    memzero
    ret