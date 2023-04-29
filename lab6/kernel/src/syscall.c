#include "bcm2837/rpi_mbox.h"
#include "syscall.h"
#include "sched.h"
#include "uart1.h"
#include "u_string.h"
#include "exception.h"
#include "memory.h"
#include "mbox.h"
#include "signal.h"
#include "mmu.h"
#include "cpio.h"
#include "dtb.h"

extern void* CPIO_DEFAULT_START;
extern thread_t *curr_thread;
extern thread_t threads[PIDMAX + 1];

// trap is like a shared buffer for user space and kernel space
// Because general-purpose registers are used for both arguments and return value,
// We may receive the arguments we need, and overwrite them with return value.

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
    kfree(curr_thread->data);
    curr_thread->datasize = get_file_size((char *)name);
    char *new_data = get_file_start((char *)name);
    curr_thread->data = kmalloc(curr_thread->datasize);

    //remap code
    mappages(PHYS_TO_VIRT(curr_thread->context.ttbr0_el1), 0x0, curr_thread->datasize, (size_t)VIRT_TO_PHYS(curr_thread->data));

    for (unsigned int i = 0; i < curr_thread->datasize; i++)
    {
        curr_thread->data[i] = new_data[i];
    }

    // inital signal handler
    for (int i = 0; i <= SIGNAL_MAX; i++)
    {
        curr_thread->signal_handler[i] = signal_default_handler;
    }

    tpf->elr_el1 = 0;
    tpf->sp_el0 = 0xfffffffff000;
    tpf->x0 = 0;
    return 0;
}

int fork(trapframe_t *tpf)
{
    lock();
    thread_t *newt = thread_create(curr_thread->data, curr_thread->datasize);

    //copy signal handler
    for (int i = 0; i <= SIGNAL_MAX;i++)
    {
        newt->signal_handler[i] = curr_thread->signal_handler[i];
    }

    mappages(newt->context.ttbr0_el1, 0x3C000000L, 0x3000000L, 0x3C000000L);
    mappages(newt->context.ttbr0_el1, 0x3F000000L, 0x1000000L, 0x3F000000L);
    mappages(newt->context.ttbr0_el1, 0x40000000L, 0x40000000L, 0x40000000L);

    // remap code and stack
    mappages(newt->context.ttbr0_el1, 0xffffffffb000, 0x4000, (size_t)VIRT_TO_PHYS(newt->stack_alloced_ptr));
    mappages(newt->context.ttbr0_el1, 0x0, newt->datasize, (size_t)VIRT_TO_PHYS(newt->data));

    int parent_pid = curr_thread->pid;

    //copy data into new process
    for (int i = 0; i < newt->datasize; i++)
    {
        newt->data[i] = curr_thread->data[i];
    }

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

    void *temp_ttbr0_el1 = newt->context.ttbr0_el1;
    newt->context = curr_thread->context;
    newt->context.ttbr0_el1 = VIRT_TO_PHYS(temp_ttbr0_el1);
    newt->context.fp += newt->kernel_stack_alloced_ptr - curr_thread->kernel_stack_alloced_ptr; // move fp
    newt->context.sp += newt->kernel_stack_alloced_ptr - curr_thread->kernel_stack_alloced_ptr; // move kernel sp

    unlock();

    tpf->x0 = newt->pid;
    return newt->pid;

child:
    tpf->x0 = 0;
    return 0;           // pid = 0
}

void exit(trapframe_t *tpf, int status)
{
    thread_exit();
}

int syscall_mbox_call(trapframe_t *tpf, unsigned char ch, unsigned int *mbox_user)
{
    lock();

    unsigned int size_of_mbox = mbox_user[0];
    memcpy((char *)pt, mbox_user, size_of_mbox);
    mbox_call(MBOX_TAGS_ARM_TO_VC, (unsigned int)((unsigned long)&pt));
    memcpy(mbox_user, (char *)pt, size_of_mbox);

    tpf->x0 = 8;

    unlock();
    return 0;
}

void kill(trapframe_t *tpf, int pid)
{
    if ( pid < 0 || pid >= PIDMAX || !threads[pid].isused) return;
    lock();
    threads[pid].iszombie = 1;
    unlock();
    schedule();
}

void signal_register(int signal, void (*handler)())
{
    if (signal > SIGNAL_MAX || signal < 0) return;
    curr_thread->signal_handler[signal] = handler;
}

void signal_kill(int pid, int signal)
{
    if (pid > PIDMAX || pid < 0 || !threads[pid].isused)return;
    lock();
    threads[pid].sigcount[signal]++;
    unlock();
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
