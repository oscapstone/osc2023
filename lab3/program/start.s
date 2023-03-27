.section ".text"

.global _start

_start:
    mov    x0, 0

_increase:
    add    x0, x0, 1
    svc    0
    cmp    x0, 5
    blt    _increase

_halt:
    b      _halt
