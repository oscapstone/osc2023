.section ".text"

syscall:
    mov x8, x0
    mov x0, x1
    mov x1, x2
    mov x2, x3
    mov x3, x4
    mov x4, x5
    mov x5, x6
    mov x6, x7

    svc 0

    cmp x0, -4096
    b.hi .Lsyscall_failed

    ret

.Lsyscall_failed:
    neg x0, x0
    ldr x1, =errno
    str x0, [x1]

    mov x0, -1
    ret

.type syscall, function
.size syscall, . - syscall
.global syscall
