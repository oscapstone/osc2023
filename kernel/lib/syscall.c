#include "syscall.h"
#include "sched.h"
#include "stddef.h"
#include "uart.h"
#include "cpio.h"
#include "irq.h"
#include "malloc.h"
#include "mbox.h"
#include "signal.h"
#include "current.h"
#include "mmu.h"
#include "string.h"
#include "filesystem/vfs.h"
#include "filesystem/dev_framebuffer.h"

int getpid(trapframe_t *tpf)
{
    tpf->x0 = curr_thread->pid;
    return curr_thread->pid;
}

size_t uartread(trapframe_t *tpf, char buf[], size_t size)
{
    int i = 0;
    for (int i = 0; i < size; i++)
        buf[i] = uart_async_getc();

    tpf->x0 = i;
    return i;
}

size_t uartwrite(trapframe_t *tpf, const char buf[], size_t size)
{
    int i = 0;
    for (int i = 0; i < size; i++)
        uart_async_putc(buf[i]);

    tpf->x0 = i;
    return i;
}

// In this lab, you won’t have to deal with argument passing
int exec(trapframe_t *tpf, const char *name, char *const argv[])
{
    // free alloced area and vma struct
    list_head_t *pos = curr_thread->vma_list.next;
    while (pos != &curr_thread->vma_list)
    {   
        // is_alloced -> malloc
        if (((vm_area_struct_t *)pos)->is_alloced)
            free((void *)PHYS_TO_VIRT(((vm_area_struct_t *)pos)->phys_addr));

        list_head_t *next_pos = pos->next;
        free(pos);
        pos = next_pos;
    }

    // exec program
    INIT_LIST_HEAD(&curr_thread->vma_list);
    // change abs_path
    char abs_path[MAX_PATH_NAME];
    strcpy(abs_path, name);
    path_to_absolute(abs_path, curr_thread->cwd);
    // get file from filesystem
    struct vnode *target_file;
    vfs_lookup(abs_path, &target_file);
    curr_thread->datasize = target_file->f_ops->getsize(target_file);
    curr_thread->data = malloc(curr_thread->datasize);
    curr_thread->user_sp = malloc(USTACK_SIZE);

    // flush tlb
    // malloc new ttbr0_el1
    __asm__ __volatile__("dsb ish\n\t"); // ensure write has completed
    free_page_tables(curr_thread->context.ttbr0_el1, 0);
    // new ttbr0_el1
    memset(PHYS_TO_VIRT(curr_thread->context.ttbr0_el1), 0, 0x1000);
    __asm__ __volatile__("tlbi vmalle1is\n\t" // invalidate all TLB entries
                         "dsb ish\n\t"        // ensure completion of TLB invalidatation
                         "isb\n\t");          // clear pipeline

    // remap code
    add_vma(curr_thread, 0, curr_thread->datasize, (size_t)VIRT_TO_PHYS(curr_thread->data), 7, 1);
    // remap stack
    add_vma(curr_thread, 0xffffffffb000, 0x4000, (size_t)VIRT_TO_PHYS(curr_thread->user_sp), 7, 1);
    // videocore memory (mailbox)
    add_vma(curr_thread, 0x3C000000L, 0x3000000L, 0x3C000000L, 3, 0);
    // for signal wrapper
    add_vma(curr_thread, USER_SIG_WRAPPER_VIRT_ADDR_ALIGNED, 0x2000, (size_t)VIRT_TO_PHYS(signal_handler_wrapper), 5, 0);

    // copy file into data
    //memcpy(curr_thread->data, new_data, curr_thread->datasize);
    struct file *f;
    // copy file frome filesystem
    vfs_open(abs_path, 0, &f);
    vfs_read(f, curr_thread->data, curr_thread->datasize);
    vfs_close(f);
    // clear signal handler
    for (int i = 0; i <= SIGNAL_MAX; i++)
        curr_thread->signal_handler[i] = signal_default_handler;

    // set elr sp in user space
    tpf->elr_el1 = 0;
    tpf->sp_el0 = 0xfffffffff000;
    tpf->x0 = 0;
    return 0;
}

int fork(trapframe_t *tpf)
{       
    // addvma : thread
    // code : 0x0
    // user stack : 0xffffffffb000 
    lock();
    thread_t *child_thread = thread_create(curr_thread->data, curr_thread->datasize);

    // copy signal handler
    for (int i = 0; i <= SIGNAL_MAX; i++)
        child_thread->signal_handler[i] = curr_thread->signal_handler[i];

    // copy file handle
    for (int i = 0; i <= MAX_FD; ++i)
     {
         if (curr_thread->fdt[i])
         {
             child_thread->fdt[i] = malloc(sizeof(struct file));
             *child_thread->fdt[i] = *curr_thread->fdt[i];
         }
     }
    // copy parent's vma except signal wrapper and video core memory
    list_head_t *pos;
    list_for_each(pos, &curr_thread->vma_list)
    {
        vm_area_struct_t *cur_vma = (vm_area_struct_t *)pos;
        if (cur_vma->virt_addr == USER_SIG_WRAPPER_VIRT_ADDR_ALIGNED || cur_vma->virt_addr == 0x3C000000)
            continue;

        char *new_alloc = malloc(cur_vma->area_size); // original buddy system buggy.
        add_vma(child_thread, cur_vma->virt_addr, cur_vma->area_size, (size_t)VIRT_TO_PHYS(new_alloc), cur_vma->rwx, 1);

        memcpy(new_alloc, (void *)PHYS_TO_VIRT(cur_vma->phys_addr), cur_vma->area_size);
    }
    // video core and signal wrapper are not malloc
    // every video core memory and signal wrapper of thread point to the same memory
    // videocore memory (mailbox)
    add_vma(child_thread, 0x3C000000L, 0x3000000L, 0x3C000000L, 3, 0);
    // for signal wrapper
    add_vma(child_thread, USER_SIG_WRAPPER_VIRT_ADDR_ALIGNED, 0x2000, (size_t)VIRT_TO_PHYS(signal_handler_wrapper), 5, 0); // for signal wrapper

    int parent_pid = curr_thread->pid;

    // copy stack into new process
    memcpy(child_thread->kernel_sp, curr_thread->kernel_sp, KSTACK_SIZE);

    store_context(current_ctx);
    // for child
    if (parent_pid != curr_thread->pid)
        goto child;

    child_thread->context.x19 = curr_thread->context.x19;
    child_thread->context.x20 = curr_thread->context.x20;
    child_thread->context.x21 = curr_thread->context.x21;
    child_thread->context.x22 = curr_thread->context.x22;
    child_thread->context.x23 = curr_thread->context.x23;
    child_thread->context.x24 = curr_thread->context.x24;
    child_thread->context.x25 = curr_thread->context.x25;
    child_thread->context.x26 = curr_thread->context.x26;
    child_thread->context.x27 = curr_thread->context.x28;
    child_thread->context.x28 = curr_thread->context.x28;
    child_thread->context.fp = child_thread->kernel_sp + curr_thread->context.fp - curr_thread->kernel_sp; // move fp
    child_thread->context.lr = curr_thread->context.lr;
    // set up child thread (not copy from parent)
    child_thread->context.sp = child_thread->kernel_sp + curr_thread->context.sp - curr_thread->kernel_sp; // move kernel sp
    child_thread->context.ttbr0_el1 = VIRT_TO_PHYS(child_thread->context.ttbr0_el1);

    unlock();

    tpf->x0 = child_thread->pid;
    return child_thread->pid;

child:
    tpf->x0 = 0;
    return 0;
}

void exit(trapframe_t *tpf, int status)
{
    thread_exit();
}

int syscall_mbox_call(trapframe_t *tpf, unsigned char ch, unsigned int *mbox_user)
{
    lock();
    // move user space mbox message to kernel space
    unsigned int size_of_mbox = mbox_user[0];
    memcpy((char *)mbox, mbox_user, size_of_mbox);
    mbox_call(ch);
    memcpy(mbox_user, (char *)mbox, size_of_mbox);

    tpf->x0 = 8;
    unlock();
    return 0;
}

void kill(trapframe_t *tpf, int pid)
{
    lock();
    if (pid >= PIDMAX || pid < 0 || threads[pid].status == NEW)
    {
        unlock();
        return;
    }
    threads[pid].status = DEAD;
    unlock();
    schedule();
}

void signal_register(int signal, void (*handler)())
{
    if (signal > SIGNAL_MAX || signal < 0)
        return;

    curr_thread->signal_handler[signal] = handler;
}

void signal_kill(int pid, int signal)
{
    if (pid > PIDMAX || pid < 0 || threads[pid].status == NEW)
        return;

    lock();
    threads[pid].sigcount[signal]++;
    unlock();
}

void sigreturn(trapframe_t *tpf)
{
    load_context(&curr_thread->signal_saved_context);
}

// only need to implement the anonymous page mapping in this Lab.
void *sys_mmap(trapframe_t *tpf, void *addr, size_t len, int prot, int flags, int fdt, int file_offset)
{
    // relocate to zero
    if (len + (unsigned long)addr >= 0xfffffffff000L)
        addr = 0L;

    // allign
    len = len % 0x1000 ? len + (0x1000 - len % 0x1000) : len; // rounds up
    addr = (unsigned long)addr % 0x1000 ? addr + (0x1000 - (unsigned long)addr % 0x1000) : addr;

    // check if any vma overlap the page
    list_head_t *pos;
    vm_area_struct_t *the_area_ptr = 0;
    list_for_each(pos, &curr_thread->vma_list)
    {
        if (!(((vm_area_struct_t *)pos)->virt_addr >= (unsigned long)(addr + len) || ((vm_area_struct_t *)pos)->virt_addr + ((vm_area_struct_t *)pos)->area_size <= (unsigned long)addr))
        {
            the_area_ptr = (vm_area_struct_t *)pos;
            break;
        }
    }

    // overlapped -> make the end as start
    if (the_area_ptr)
    {
        tpf->x0 = (unsigned long)sys_mmap(tpf, (void *)(the_area_ptr->virt_addr + the_area_ptr->area_size), len, prot, flags, fdt, file_offset);
        return (void *)tpf->x0;
    }
    // if not overlapped add this page to vma
    add_vma(curr_thread, (unsigned long)addr, len, VIRT_TO_PHYS((unsigned long)malloc(len)), prot, 1);
    tpf->x0 = (unsigned long)addr;
    return (void *)tpf->x0;
}

// syscall number : 11
int sys_open(trapframe_t *tpf, const char *pathname, int flags)
{
    char abs_path[MAX_PATH_NAME];
    strcpy(abs_path, pathname);
    // get absolute name
    path_to_absolute(abs_path, curr_thread->cwd);
    for (int i = 0; i < MAX_FD; i++)
    {
        if(!curr_thread->fdt[i])
        {
            // open
            if(vfs_open(abs_path, flags, &curr_thread->fdt[i])!=0)
            {
                break;
            }

            tpf->x0 = i;
            return i;
        }
    }

    tpf->x0 = -1;
    return -1;
}

// syscall number : 12
int sys_close(trapframe_t *tpf, int fd)
{
    if(curr_thread->fdt[fd])
    {   
        // free current file handle
        vfs_close(curr_thread->fdt[fd]);
        curr_thread->fdt[fd] = 0;
        tpf->x0 = 0;
        return 0;
    }

    tpf->x0 = -1;
    return -1;
}

// syscall number : 13
// remember to return read size or error code
long sys_write(trapframe_t *tpf, int fd, const void *buf, unsigned long count)
{   
    // write to fdt
    if (curr_thread->fdt[fd])
    {
        tpf->x0 = vfs_write(curr_thread->fdt[fd], buf, count);
        return tpf->x0;
    }

    tpf->x0 = -1;
    return tpf->x0;
}

// syscall number : 14
// remember to return read size or error code
long sys_read(trapframe_t *tpf, int fd, void *buf, unsigned long count)
{
    // read to fdt
    if (curr_thread->fdt[fd])
    {
        tpf->x0 = vfs_read(curr_thread->fdt[fd], buf, count);
        return tpf->x0;
    }

    tpf->x0 = -1;
    return tpf->x0;
}

// syscall number : 15
// you can ignore mode, since there is no access control
int sys_mkdir(trapframe_t *tpf, const char *pathname, unsigned mode)
{
    char abs_path[MAX_PATH_NAME];
    strcpy(abs_path, pathname);
    // remeber to change absolute path
    path_to_absolute(abs_path, curr_thread->cwd);
    tpf->x0 = vfs_mkdir(abs_path);
    return tpf->x0;
}

// syscall number : 16
// you can ignore arguments other than target (where to mount) and filesystem (fs name)
int sys_mount(trapframe_t *tpf, const char *src, const char *target, const char *filesystem, unsigned long flags, const void *data)
{
    char abs_path[MAX_PATH_NAME];
    strcpy(abs_path, target);
    path_to_absolute(abs_path, curr_thread->cwd);

    tpf->x0 = vfs_mount(abs_path,filesystem);
    return tpf->x0;
}

// syscall number : 17
int sys_chdir(trapframe_t *tpf, const char *path)
{
    char abs_path[MAX_PATH_NAME];
    strcpy(abs_path, path);
    path_to_absolute(abs_path, curr_thread->cwd);
    // change dir
    strcpy(curr_thread->cwd, abs_path);

    return 0;
}

// syscall number : 18
// you only need to implement seek set
long sys_lseek64(trapframe_t *tpf,int fd, long offset, int whence)
{
    // get offset
    tpf->x0 = vfs_lseek64(curr_thread->fdt[fd],offset,whence);
    return tpf->x0;
}
extern unsigned int height;
extern unsigned int isrgb;
extern unsigned int pitch;
extern unsigned int width;
int sys_ioctl(trapframe_t *tpf, int fb, unsigned long request, void *info)
{
    if(request == 0)
    {
        // set framebuffer info
        struct framebuffer_info *fb_info = info;
        fb_info->height = height;
        fb_info->isrgb = isrgb;
        fb_info->pitch = pitch;
        fb_info->width = width;
    }

    tpf->x0 = 0;
    return tpf->x0;
}