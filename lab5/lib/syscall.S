#include "syscall.h"


/* it just stores syscall number in the w8 register and generates a synchronous exception by executing svc instruction. 
egisters x0 — x7are used for syscall arguments and
x8 is used to store syscall number, */
.globl getpid
getpid:
    mov w8, #SYS_GETPID_NUM
    svc #0
    ret

.globl uart_read
uart_read:
    mov w8, #SYS_UARTREAD_NUM
    svc #0
    ret

.globl uart_write
uart_write:
    mov w8, #SYS_UARTWRITE_NUM
    svc #0
    ret

.globl exec
exec:
    mov w8, #SYS_EXEC_NUM
    svc #0
    ret

.globl fork
fork:
    mov w8, #SYS_FORK_NUM
    svc #0
    ret

.globl exit
exit:
    mov w8, #SYS_EXIT_NUM
    svc #0
    ret

.globl mbox_call
mbox_call:
    mov w8, #SYS_MBOXCALL_NUM
    svc #0
    ret

.globl kill
kill:
    mov w8, #SYS_KILL_NUM
    svc #0
    ret
