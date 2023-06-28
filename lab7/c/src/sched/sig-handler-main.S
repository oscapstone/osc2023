.section ".text"

sig_handler_main:
    blr x0
    mov x8, 10 // SYS_sigreturn
    svc 0

.type sig_handler_main, function
.size sig_handler_main, . - sig_handler_main
.global sig_handler_main
