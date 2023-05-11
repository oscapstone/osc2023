#include "sys.h"

.globl call_sys_write
call_sys_write:
    mov w8, #SYS_WRITE_NUMBER
    svc #0
    ret

.globl uartwrite
uartwrite:
    mov w8, #SYS_UARTWRITE_NUMBER
    svc #0
    ret

.globl uartread
uartread:
    mov w8, #SYS_UARTREAD_NUMBER
    svc #0
    ret

.globl getpid
getpid:
    mov w8, #SYS_GETPID_NUMBER
    svc #0
    ret

.globl fork
fork:
    mov w8, #SYS_FORK_NUMBER
    svc #0
    ret

.globl exec
exec:
    mov w8, #SYS_EXEC_NUMBER
    svc #0
    ret

.globl exit
exit:
    mov w8, #SYS_EXIT_NUMBER
    svc #0
    ret

.global mbox_call
mbox_call:
	mov w8, SYS_MBOX_NUMBER
	svc #0
	ret

.global kill
kill:
	mov w8, SYS_KILL_NUMBER
	svc #0
	ret

.globl call_sys_malloc
call_sys_malloc:
    mov w8, #SYS_MALLOC_NUMBER
    svc #0
    ret

.globl call_sys_coreTimer_on
call_sys_coreTimer_on:
    mov w8, #SYS_CORETIMER_On
    svc #0
    ret

.globl call_sys_coreTimer_off
call_sys_coreTimer_off:
    mov w8, #SYS_CORETIMER_OFF
    svc #0
    ret


