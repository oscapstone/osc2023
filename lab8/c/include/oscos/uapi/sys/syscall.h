#ifndef OSCOS_UAPI_SYS_SYSCALL_H
#define OSCOS_UAPI_SYS_SYSCALL_H

#define SYS_getpid 0
#define SYS_uart_read 1
#define SYS_uart_write 2
#define SYS_exec 3
#define SYS_fork 4
#define SYS_exit 5
#define SYS_mbox_call 6
#define SYS_kill 7
#define SYS_signal 8
#define SYS_signal_kill 9
#define SYS_mmap 10
#define SYS_open 11
#define SYS_close 12
#define SYS_write 13
#define SYS_read 14
#define SYS_mkdir 15
#define SYS_mount 16
#define SYS_chdir 17
#define SYS_lseek64 18
#define SYS_ioctl 19
#define SYS_sync 20
#define SYS_sigreturn 21

#endif
