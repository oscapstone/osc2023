#include "syscall.h"

.global getpid
getpid:
    mov x8, SYS_GETPID
    svc 0
    ret

.global uart_read
uart_read:
    mov x8, SYS_UART_READ
    svc 0
    ret

.global uart_write
uart_write:
    mov x8, SYS_UART_WRITE
    svc 0
    ret

.global exec
exec:
    mov x8, SYS_EXEC
    svc 0
    ret

.global exit
exit:
    mov x8, SYS_EXIT
    svc 0
    ret

.global fork
fork:
    mov x8, SYS_FORK
    svc 0
    ret
.global mbox_call
mbox_call:
    mov x8, SYS_MBOX_CALL
    svc 0
    ret

.global kill
kill:
    mov x8, SYS_KILL
    svc 0
    ret

.global schedule
schedule:
    mov x8, SYS_SCHEDULE
    svc 0
    ret

.global enable_core_timer
enable_core_timer:
    mov x8, SYS_ENABLE_CORE_TIMER
    svc 0
    ret

.global disable_core_timer
disable_core_timer:
    mov x8, SYS_DISABLE_CORE_TIMER
    svc 0
    ret

.global set_timeout
set_timeout:
    mov x8, SYS_SET_TIMEOUT
    svc 2
    ret
