#include "stdlib.h"
#include "thread.h"
#include "mini_uart.h"
#include "page_alloc.h"
#include "read_cpio.h"
#include "utils.h"
#include "peripherals/mbox_call.h"
#include "peripherals/gpio.h"
#include "my_signal.h"
#include "vfs.h"

extern task_struct *get_current();
extern void set_switch_timer();
extern void enable_interrupt();
extern void disable_interrupt();
extern void core_timer_enable();
extern void switch_to(task_struct *, task_struct *);
extern task_struct kernel_thread;
extern struct list_head task_rq_head;
extern struct list_head task_zombieq_head;

int getpid()
{
    int ret = get_current()->thread_info->id;
    return ret;
}

size_t uart_read(char buf[], size_t size)
{
    for (int i = 0; i < size; i++)
    {
        buf[i] = uart_recv();
        if (buf[i] == '\n' || buf[i] == '\r')
            return i;
    }
    return size;
}

size_t uart_write(const char buf[], size_t size)
{
    for (int i = 0; i < size; i++)
    {
        if (buf[i] == '\0')
            return i;
        uart_send(buf[i]);
    }
    return size;
}

int exec(const char *name, char *const argv[])
{
    task_struct *current = get_current();
    unsigned long target_addr = current->usrpgm_load_addr;
    unsigned long target_sp = current->ustack_start + MIN_PAGE_SIZE;

    set_switch_timer();
    core_timer_enable();
    enable_interrupt();

    load_userprogram(name, (char *)target_addr);

    asm volatile(
        "mov x0, 0x0\n\t" // EL0t
        "msr spsr_el1, x0\n\t"
        "mov x0, %0\n\t"
        "msr elr_el1, x0\n\t"
        "mov x0, %1\n\t"
        "msr sp_el0, x0\n\t"
        "eret"
        :
        : "r"(target_addr), "r"(target_sp)
        : "x0");

    return 0;
}

int fork()
{
    schedule();
    return get_current()->thread_info->child_id;
}

void exit(int status)
{
    task_struct *current = (task_struct *)get_current();
    current->status = ZOMBIE;
    INIT_LIST_HEAD(&current->list);
    list_add_tail(&current->list, &task_zombieq_head);
    // schedule();
    switch_to(current, &kernel_thread);
}

int mbox_call_u(unsigned char ch, unsigned int *mbox)
{
    unsigned int r = (((unsigned int)((unsigned long)mbox) & ~0xF) | (ch & 0xF));

    /* wait until we can write to the mailbox */
    while (1)
    {
        if (!(get32(MBOX_STATUS) & MBOX_FULL))
            break;
    }

    /* write the address of our message to the mailbox with channel identifier */
    put32(MBOX_WRITE, r);

    /* now wait for the response */
    while (1)
    {
        /* is there a response? */
        while (1)
        {
            if (!(get32(MBOX_STATUS) & MBOX_EMPTY))
                break;
        }

        /* is it a response to our message? */
        if (r == get32(MBOX_READ))
            /* is it a valid successful response? */
            return mbox[1] == MBOX_RESPONSE;
    }

    return 0;
}

void kill(int pid)
{
    task_struct *tmp = get_current();
    if (tmp->thread_info->id == pid)
        exit(0);

    struct list_head *iter = &task_rq_head;
    struct list_head *start = &task_rq_head;
    while (iter->next != start)
    {
        iter = iter->next;
        tmp = container_of(iter, task_struct, list);
        if (tmp->thread_info->id == pid)
        {
            tmp->status = ZOMBIE;
            return;
        }
    }
}

void signal(int SIGNAL, void (*handler)())
{
    printf("[info] Called signal()\n");
    task_struct *cur = get_current();

    custom_signal *new = (custom_signal *)my_malloc(sizeof(custom_signal));
    new->sig_num = SIGNAL;
    new->handler = handler;
    INIT_LIST_HEAD(&new->list);

    if (!cur->custom_signal)
        cur->custom_signal = new;
    else
        list_add_tail(&cur->custom_signal->list, &new->list);
}

int open(char *pathname, int flags)
{
    task_struct *cur = get_current();
    int fd = thread_get_idle_fd(cur);
    if (fd < 0)
    {
        printf("Error, priv_open(), cannot open more file for pid=%d\r\n", cur->thread_info->id);
        return -1;
    }

    file_t *target;
    int ret = vfs_open(pathname, flags, &target);

    if (ret != 0)
        return ret;

    cur->fd_table[fd] = target;

    return fd;
}

int close(int fd)
{
    task_struct *cur = get_current();
    int ret = vfs_close(cur->fd_table[fd]);
    return ret;
}

long write(int fd, void *buf, unsigned long count)
{
    task_struct *cur = get_current();
    int ret = vfs_write(cur->fd_table[fd], buf, count);
    return ret;
}

long read(int fd, void *buf, unsigned long count)
{
    task_struct *cur = get_current();
    int ret = vfs_read(cur->fd_table[fd], buf, count);
    return ret;
}

int mkdir(char *pathname, unsigned mode)
{
    int ret = vfs_mkdir(pathname);
    return ret;
}

int mount(char *src, char *target, char *filesystem, unsigned long flags, void *data)
{
    int ret = vfs_mount(target, filesystem);
    return ret;
}

int chdir(char *path)
{
    char pathname[256];
    handle_path(path, pathname);
    int ret = vfs_cd(pathname);
    return ret;
}

long lseek(int fd, long offset, int whence)
{
    task_struct *cur = get_current();
    int ret = vfs_lseek(cur->fd_table[fd], offset, whence);
    return ret;
}

int ioctl(int fd, unsigned long request, unsigned long arg)
{
    task_struct *cur = get_current();
    int ret = vfs_ioctl(cur->fd_table[fd], request, arg);
    return ret;
}