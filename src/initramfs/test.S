.section ".data"
    bad: .string "BAD: "
    hello: .string "hello"

.section ".text"
    sub sp, sp, 16
    stp x29, x30, [sp]
    adr x1, bad
    mov x2, #5
    mov x0, 2
    svc 0
    mov x0, 4
    svc 0
    ldp x29, x30, [sp]
    ret
    // ldr x3, [sp]
    // strb w3, [x1]
    // mov x0, 2
    // svc 0
// func:
    // b func

