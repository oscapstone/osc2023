.global syscall_sigreturn
syscall_sigreturn:
    mov x8, #10
    svc 0