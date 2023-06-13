#include "peripherals/syscall.h"

.globl getpid
getpid:
    mov x8, #SYSCALL_NUM_GET_PID
    svc 0
    ret

.globl uartread
uartread:
    mov x8, #SYSCALL_NUM_UART_READ
    svc 0
    ret

.globl uartwrite
uartwrite:
    mov x8, #SYSCALL_NUM_UART_WRITE
    svc 0
    ret

/*
 * Returns -1 on failure, 0 in succeed
 */
.globl exec
exec:
    mov x8, #SYSCALL_NUM_EXEC
    svc 0
    ret

.globl fork
fork:
    mov x8, #SYSCALL_NUM_FORK
    svc 0
    ret

.globl exit
exit:
    mov x8, #SYSCALL_NUM_EXIT
    svc 0
    ret

.globl mbox_call
mbox_call:
    mov x8, #SYSCALL_NUM_MBOX_CALL
    svc 0
    ret

.globl kill
kill:
    mov x8, #SYSCALL_NUM_KILL
    svc 0
    ret