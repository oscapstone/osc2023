.section ".text.boot"
.global _start
_start:
    mov x0, 0
1:
    add x0, x0, 1
    mov x8, #0
    svc 0
    mov x8, #4
    svc 0
    mov x8, #9
    svc 0
    mov x8, #5
    svc 0
    blt 1b
1:
    b 1b

get_pid:
    mov x8, #0
    svc 0
    ret