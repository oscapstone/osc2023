#include "process.h"
#include "thread.h"
#include "syscall.h"
#include "interrupt.h"
#include "peripherals/mini_uart.h"
#include "mbox/mbox.h"
#include "fs/vfs.h"
#include "fs/fat32.h"

int sys_get_pid() {
    struct Process_t *proc = process_get_current();

    return proc->pid;
}
uint64_t sys_uartread(char buf[], uint64_t size) {
    uint64_t ret = async_uart_rx_buf(buf, size);
    return ret;
}
uint64_t sys_uartwrite(const char buf[], uint64_t size) {
    uint64_t ret = uart_send_n(buf, size);
    return ret;
}
int sys_exec(const char* name, char *const argv[]) {
    uint64_t ret = process_exec(name, argv, 0);
    return ret;
}
int sys_fork(struct Trapframe_t *frame) {
    uint64_t ret = process_fork(frame);

    return ret;
}
void sys_exit(int status) {
    process_exit(status);
    return; 
}

int sys_mbox_call(unsigned char ch, unsigned int *mbox) {
    uint64_t x = mbox_call(ch, mbox);
    return x;
}

void sys_kill(int pid) {
    signal_kill(pid, 9);
    return;
}

int sys_open(const char *pathname, int flags) {
    struct file *f;
    int err = vfs_open(pathname, flags, &f);
    if(err) return err;
    struct Process_t *cur_proc = process_get_current();
    for(int i = 0; i < P_FD_MAX; i ++) {
        if(cur_proc->files[i] == NULL) {
            cur_proc->files[i] = f;
            return i;
        }
    }
    return -1;
}

int sys_close(int fd) {
    struct file *f;
    struct Process_t *cur_proc = process_get_current();
    int err = vfs_close(cur_proc->files[fd]);
    if(err) return err;
    cur_proc->files[fd] = NULL;
    return err;
}

// syscall number : 13
// remember to return read size or error code
long sys_write(int fd, const void *buf, unsigned long count) {
    struct Process_t *cur_proc = process_get_current();
    int err = vfs_write(cur_proc->files[fd], buf, count);
    return err;
}

// syscall number : 14
// remember to return read size or error code
long sys_read(int fd, void *buf, unsigned long count) {
    struct Process_t *cur_proc = process_get_current();
    int err = vfs_read(cur_proc->files[fd], buf, count);
    return err;
}

// syscall number : 15
// you can ignore mode, since there is no access control
int sys_mkdir(const char *pathname, unsigned mode) {
    int err = vfs_mkdir(pathname);
    return err;
}


// syscall number : 16
// you can ignore arguments other than target (where to mount) and filesystem (fs name)
int sys_mount(const char *src, const char *target, const char *filesystem, unsigned long flags, const void *data) {
    int err = vfs_mount(target, filesystem);
    return err;
}


// syscall number : 17
int sys_chdir(const char *path) {
    struct vnode *dir;
    int err = vfs_lookup(path, &dir);

    if(err != 0) {
        return err;
    } else {
        struct Process_t *proc = process_get_current();
        proc->cur_vnode = dir;
        proc->mnt = dir->mount;
    }
    return err;
}

int sys_lseek(int fd, long offset, int whence) {
    struct Process_t *proc = process_get_current();
    struct file *f = proc->files[fd];

    return f->f_ops->lseek64(f, offset, whence);
}

int sys_ioctl(int fd, unsigned long request, void *buf) {
    struct Process_t *proc = process_get_current();
    struct file *f = proc->files[fd];

    return f->f_ops->ioctl(f, request, buf);
}

void syscall_handler(struct Trapframe_t *frame) {

    switch(frame->x8) {
        case 0:
            frame->x0 = sys_get_pid();
            break;
        case 1:
            frame->x0 = sys_uartread(frame->x0, frame->x1);
            break;
        case 2:
            frame->x0 = sys_uartwrite(frame->x0, frame->x1);
            break;
        case 3:
            frame->x0 = sys_exec(frame->x0, frame->x1);
            break;
        case 4:
            unsigned long long val = sys_fork(frame);
            frame->x0 = val;
            break;
        case 5:
            sys_exit(frame->x1);
            break;
        case 6:
            frame->x0 = sys_mbox_call(frame->x0, frame->x1);
            break;
        case 7:
            sys_kill(frame->x0);
            break;

        case 8:
            signal_register(frame->x0, frame->x1);
            break;
        case 9:
            signal_kill(frame->x0, frame->x1);
            break;
        case 10:
            signal_sigreturn();
            break;

        case 11:
            frame->x0 = sys_open(frame->x0, frame->x1);
            break;
        case 12:
            frame->x0 = sys_close(frame->x0);
            break;
        case 13:
            frame->x0 = sys_write(frame->x0, frame->x1, frame->x2);
            break;
        case 14:
            frame->x0 = sys_read(frame->x0, frame->x1, frame->x2);
            break;

        case 15:
            frame->x0 = sys_mkdir(frame->x0, frame->x1);
            break;

        case 16:
   
            sys_mount(frame->x0, frame->x1, frame->x2, frame->x3, frame->x4);
            break;

        case 17:
            frame->x0 = sys_chdir(frame->x0);
            break;

        case 18:
            frame->x0 = sys_lseek(frame->x0, frame->x1, frame->x2);
            break;

        case 19:
            frame->x0 = sys_ioctl(frame->x0, frame->x1, frame->x2);
            break;
        
        case 20:
            fat32_sync();

    }
  
}
