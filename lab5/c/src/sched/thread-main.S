.section ".text"

thread_main:
    mov x0, x20
    blr x19
    b thread_exit

.size thread_main, . - thread_main
.type thread_main, function
.global thread_main
