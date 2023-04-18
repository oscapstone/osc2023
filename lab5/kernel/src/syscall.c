#include "bcm2837/rpi_mbox.h"
#include "syscall.h"
#include "sched.h"
#include "uart1.h"
#include "u_string.h"
#include "exception.h"
#include "memory.h"
#include "mbox.h"
#include "signal.h"

#include "cpio.h"
#include "dtb.h"

extern void* CPIO_DEFAULT_START;
extern thread_t *curr_thread;

extern thread_t threads[PIDMAX + 1];

int getpid(trapframe_t *tpf)
{
    tpf->x0 = curr_thread->pid;
    return curr_thread->pid;
}

size_t uartread(trapframe_t *tpf, char buf[], size_t size)
{
    int i = 0;
    for (int i = 0; i < size;i++)
    {
        buf[i] = uart_async_getc();
    }
    tpf->x0 = i;
    return i;
}

size_t uartwrite(trapframe_t *tpf, const char buf[], size_t size)
{
    int i = 0;
    for (int i = 0; i < size; i++)
    {
        uart_async_putc(buf[i]);
    }
    tpf->x0 = i;
    return i;
}

//In this lab, you won’t have to deal with argument passing
int exec(trapframe_t *tpf, const char *name, char *const argv[])
{
    curr_thread->datasize = get_file_size((char*)name);
    char *new_data = get_file_start((char *)name);
    for (unsigned int i = 0; i < curr_thread->datasize;i++)
    {
        curr_thread->data[i] = new_data[i];
    }

    //clear signal handler
    for (int i = 0; i <= SIGNAL_MAX; i++)
    {
        curr_thread->signal_handler[i] = signal_default_handler;
    }
    /*
    __asm__ __volatile__("mov sp, %0\n\t" ::"r"(curr_thread->stack_alloced_ptr + USTACK_SIZE));
    __asm__ __volatile__("msr elr_el1, %0\n\t" ::"r"(curr_thread->data));
    __asm__ __volatile__("eret\n\t");
    */
    tpf->elr_el1 = (unsigned long) curr_thread->data;
    tpf->sp_el0 = (unsigned long)curr_thread->stack_alloced_ptr + USTACK_SIZE;
    tpf->x0 = 0;
    return 0;
}

int fork(trapframe_t *tpf)
{
    el1_interrupt_enable();
    thread_t *newt = thread_create(curr_thread->data);
    newt->datasize = curr_thread->datasize;

    //copy signal handler
    for (int i = 0; i <= SIGNAL_MAX;i++)
    {
        newt->signal_handler[i] = curr_thread->signal_handler[i];
    }

    int parent_pid = curr_thread->pid;
    thread_t *parent_thread_t = curr_thread;

    //copy user stack into new process
    for (int i = 0; i < USTACK_SIZE; i++)
    {
        newt->stack_alloced_ptr[i] = curr_thread->stack_alloced_ptr[i];
    }

    //copy stack into new process
    for (int i = 0; i < KSTACK_SIZE; i++)
    {
        newt->kernel_stack_alloced_ptr[i] = curr_thread->kernel_stack_alloced_ptr[i];
    }

    store_context(get_current());

    //for child
    if( parent_pid != curr_thread->pid)
    {
        goto child;
    }

    newt->context = curr_thread->context;
    newt->context.fp += newt->kernel_stack_alloced_ptr - curr_thread->kernel_stack_alloced_ptr; // move fp
    newt->context.sp += newt->kernel_stack_alloced_ptr - curr_thread->kernel_stack_alloced_ptr; // move kernel sp
    el1_interrupt_disable();

    tpf->x0 = newt->pid;
    return newt->pid;

child:
    tpf = (trapframe_t*)((char *)tpf + (unsigned long)newt->kernel_stack_alloced_ptr - (unsigned long)parent_thread_t->kernel_stack_alloced_ptr); // move tpf
    tpf->sp_el0 += newt->stack_alloced_ptr - parent_thread_t->stack_alloced_ptr;
    tpf->x0 = 0;
    return 0;
}

void exit(trapframe_t *tpf, int status)
{
    thread_exit();
}

int syscall_mbox_call(trapframe_t *tpf, unsigned char ch, unsigned int *mbox)
{
    el1_interrupt_disable();
    unsigned long r = (((unsigned long)((unsigned long)mbox) & ~0xF) | (ch & 0xF));
    /* wait until we can write to the mailbox */
    do{asm volatile("nop");} while (*MBOX_STATUS & BCM_ARM_VC_MS_FULL);
    /* write the address of our message to the mailbox with channel identifier */
    *MBOX_WRITE = r;
    /* now wait for the response */
    while (1)
    {
        /* is there a response? */
        do{ asm volatile("nop");} while (*MBOX_STATUS & BCM_ARM_VC_MS_EMPTY);
        /* is it a response to our message? */
        if (r == *MBOX_READ)
        {
            /* is it a valid successful response? */
            tpf->x0 = (mbox[1] == MBOX_REQUEST_SUCCEED);
            el1_interrupt_enable();
            return mbox[1] == MBOX_REQUEST_SUCCEED;
        }
    }

    tpf->x0 = 0;
    el1_interrupt_enable();
    return 0;
}

void kill(trapframe_t *tpf, int pid)
{
    el1_interrupt_disable();
    if ( pid < 0 || pid >= PIDMAX || !threads[pid].isused)
    {
        el1_interrupt_enable();
        return;
    }
    threads[pid].iszombie = 1;
    el1_interrupt_enable();
    schedule();
}

void signal_register(int signal, void (*handler)())
{
    if (signal > SIGNAL_MAX || signal < 0)return;

    curr_thread->signal_handler[signal] = handler;
}

void signal_kill(int pid, int signal)
{
    if (pid > PIDMAX || pid < 0 || !threads[pid].isused)return;
    //lock();
    threads[pid].sigcount[signal]++;
    //unlock();
}

void sigreturn(trapframe_t *tpf)
{
    unsigned long signal_ustack = tpf->sp_el0 % USTACK_SIZE == 0 ? tpf->sp_el0 - USTACK_SIZE : tpf->sp_el0 & (~(USTACK_SIZE - 1));
    kfree((char*)signal_ustack);
    load_context(&curr_thread->signal_savedContext);
}


char* get_file_start(char *thefilepath)
{
    char *filepath;
    char *filedata;
    unsigned int filesize;
    struct cpio_newc_header *header_pointer = CPIO_DEFAULT_START;

    while (header_pointer != 0)
    {
        int error = cpio_newc_parse_header(header_pointer, &filepath, &filesize, &filedata, &header_pointer);
        //if parse header error
        if (error)
        {
            uart_puts("error");
            break;
        }

        if (strcmp(thefilepath, filepath) == 0)
        {
            return filedata;
        }

        //if this is TRAILER!!! (last of file)
        if (header_pointer == 0)
            uart_puts("execfile: %s: No such file or directory\r\n", thefilepath);
    }
    return 0;
}

unsigned int get_file_size(char *thefilepath)
{
    char *filepath;
    char *filedata;
    unsigned int filesize;
    struct cpio_newc_header *header_pointer = CPIO_DEFAULT_START;

    while (header_pointer != 0)
    {
        int error = cpio_newc_parse_header(header_pointer, &filepath, &filesize, &filedata, &header_pointer);
        //if parse header error
        if (error)
        {
            uart_puts("error");
            break;
        }

        if (strcmp(thefilepath, filepath) == 0)
        {
            return filesize;
        }

        //if this is TRAILER!!! (last of file)
        if (header_pointer == 0)
            uart_puts("execfile: %s: No such file or directory\r\n", thefilepath);
    }
    return 0;
}
