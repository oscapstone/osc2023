.section ".text"

syscall_table:
    b sys_getpid
    b sys_uart_read
    b sys_uart_write
    b sys_exec
    b sys_fork
    b sys_exit
    b sys_mbox_call
    b sys_kill
    b sys_signal
    b sys_signal_kill
    b sys_sigreturn

.size syscall_table, . - syscall_table
.global syscall_table
