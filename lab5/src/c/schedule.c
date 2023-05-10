#include "schedule.h"
#include "mm.h"
#include "printf.h"
#include "exception.h"
#include "string.h"
#include "cpio.h"
#include "syscall.h"
#include "mbox.h"

struct task_struct *task_pool[TASK_POOL_SIZE];

void delay(unsigned int count)
{
    while (count--)
    {
    }
}

void enable_preemption()
{
    int pid = get_current_task();
    task_pool[pid]->preemptible = 1;
}

void disable_preemption()
{
    int pid = get_current_task();
    task_pool[pid]->preemptible = 0;
}

void sys_getpid(struct trapframe *tf)
{
    int pid = get_current_task();
    tf->x[0] = pid;

    return;
}

void sys_exec(struct trapframe *tf)
{
    disable_preemption();

    char *program_name = (char *)tf->x[0];
    void *program_start = cpio_run_program(program_name);
    char **argv = (char **)tf->x[1];

    int argc = 0;
    while (1)
    {
        if ((char *)argv[argc] == 0)
            break;
        argc++;
    }

    char **argv_backup = km_allocation(sizeof(char *) * (argc + 1));
    for (int i = 0; i < argc; i++)
        argv_backup[i + 1] = argv[i];
    argv_backup[0] = program_name;
    argc++;

    // flush user stack
    int pid = get_current_task();
    unsigned long stack_reset = task_pool[pid]->ustack + USTACK_SIZE;
    tf->sp_el0 = stack_reset;

    // padding to multiples of 16-bytes
    tf->sp_el0 -= (16 + (argc + argc % 2) * 8);

    *(char **)tf->sp_el0 = tf->sp_el0 + 8;
    for (int i = 0; i < argc; i++)
        *(char **)(tf->sp_el0 + 8 + 8 * i) = argv_backup[i];
    *(char **)(tf->sp_el0 + 8 + 8 * argc) = 0;

    km_free(argv_backup);

    tf->x[1] = *(char ***)tf->sp_el0;

    // setup argc
    tf->sp_el0 -= 16;
    *(int *)tf->sp_el0 = argc;
    tf->x[0] = argc;

    tf->elr_el1 = program_start;

    enable_preemption();

    return;
}

void sys_exit()
{
    unsigned int current_pid = get_current_task();
    struct task_struct *current = task_pool[current_pid];

    current->state = ZOMBIE;
    sys_schedule();
}

void sys_mbox_call(struct trapframe *tf)
{
    unsigned char ch = (unsigned char)tf->x[0];
    unsigned int *mbox = (unsigned int *)tf->x[1];
    int ret = mboxc_mbox_call(ch, mbox); // defined in mbox.c
    tf->x[0] = ret;
}

void sys_kill(struct trapframe *tf)
{
    int pid = tf->x[0];
    if (task_pool[pid])
    {
        task_pool[pid]->state == ZOMBIE;
        km_free(task_pool[pid]->kstack);
        km_free(task_pool[pid]->ustack);
        km_free(task_pool[pid]);
        task_pool[pid] = NULL;
        printf("killed pid: %d\n", pid);
    }
    else
    {
        printf("kill pid: %d failed. There is no such process. \n", pid);
    }
}

void sys_signal(struct trapframe *tf)
{
    unsigned int current_pid = get_current_task();
    struct task_struct *current = task_pool[current_pid];
    current->hander = (void *)(tf->x[1]);
    printf("signal: %d, address: %x\n", tf->x[0], current->hander);
}
void sys_posix_kill(struct trapframe *tf)
{
    unsigned int pid = tf->x[0];
    unsigned int sig = tf->x[1];
    if (task_pool[pid] == NULL)
    {
        printf("kill pid: %d failed. There is no such process. \n", pid);
        return;
    }
    unsigned int current_pid = get_current_task();
    struct task_struct *current = task_pool[current_pid];
    if (current->hander == NULL)
    {
        return;
    }
    thread_create(current->hander);
}

void sys_fork(struct trapframe *tf)
{
    disable_preemption();

    int parent_pid = get_current_task();
    int child_pid = thread_create((void (*)())NULL);
    struct task_struct *parent = task_pool[parent_pid];
    struct task_struct *child = task_pool[child_pid];

    char *child_kstack = child->kstack + KSTACK_SIZE;
    char *parent_kstack = parent->kstack + KSTACK_SIZE;
    char *child_ustack = child->ustack + USTACK_SIZE;
    char *parent_ustack = parent->ustack + USTACK_SIZE;

    unsigned long kstack_offset = parent_kstack - (char *)tf;         // how many bytes the kernel stack of the parent thread have used
    unsigned long ustack_offset = parent_ustack - (char *)tf->sp_el0; // how many bytes the user stack of the parent thread have used

    for (unsigned long i = 0; i < kstack_offset; i++)
        *(child_kstack - i) = *(parent_kstack - i);
    for (unsigned long i = 0; i < ustack_offset; i++)
        *(child_ustack - i) = *(parent_ustack - i);

    // setup the kernel stack of the child thread
    child->context.sp = (unsigned long)child_kstack - kstack_offset;
    // setup the user stack of the child thread
    struct trapframe *child_tf = (struct trapframe *)child->context.sp;
    child_tf->sp_el0 = (unsigned long)child_ustack - ustack_offset;

    // setup return values
    tf->x[0] = child_pid;
    child_tf->x[0] = 0;

    enable_preemption();

    return;
}

int thread_create(void (*function)())
{
    struct task_struct *new_task = NULL;
    int pid = -1;

    for (int i = 0; i < TASK_POOL_SIZE; i++)
    {
        if (task_pool[i] == NULL)
        {
            new_task = km_allocation(sizeof(struct task_struct));
            pid = i;
            break;
        }
    }

    if (pid == -1)
    {
        printf("Failed to create a new task!\n");
        return -1;
    }

    new_task->state = RUNNING;
    new_task->pid = pid;
    new_task->preemptible = 1;
    new_task->need_schedule = 0;
    new_task->quota = TASK_QUOTA;

    new_task->context.lr = (unsigned long)return_to_user_code;
    // remember to reserve some space for the trapframe!!!!!
    new_task->context.sp = (unsigned long)km_allocation(KSTACK_SIZE) + (KSTACK_SIZE - TRAPFRAME_SIZE);
    new_task->context.fp = new_task->context.sp;

    struct trapframe *new_task_tf = (struct trapframe *)new_task->context.sp;
    new_task_tf->sp_el0 = (unsigned long)km_allocation(USTACK_SIZE) + USTACK_SIZE;
    new_task_tf->elr_el1 = (unsigned long)function;

    task_pool[pid] = new_task;
    new_task->kstack = (void *)new_task->context.sp - (KSTACK_SIZE - TRAPFRAME_SIZE);
    new_task->ustack = (void *)new_task_tf->sp_el0 - USTACK_SIZE;

    return pid;
}

void kill_zombie()
{
    for (int i = 1; i < TASK_POOL_SIZE; i++)
    {
        if (task_pool[i]->state == ZOMBIE)
        {
            printf("process ID: %d killed!\n", i);

            km_free(task_pool[i]->kstack);
            km_free(task_pool[i]->ustack);
            km_free(task_pool[i]);
            task_pool[i] = NULL;
        }
    }
}

void idle_thread()
{
    // printf("[UART_RAED] please input: \n");
    // char uart_read_buffer[6];
    // int read_size = uart_read(uart_read_buffer, 5);
    // printf("[UART_READ] %d %s\n", read_size, uart_read_buffer);

    char *uart_write_buffer = "Hello World !";
    int write_size = uart_write(uart_write_buffer, strlen(uart_write_buffer));
    printf("[UART_WRITE] %d\n", write_size);

    enable_core_timer();

    while (1)
    {
        // printf("idle thread\n");
        kill_zombie();
        schedule();
    }
}

void sys_schedule()
{
    disable_preemption();

    int prev_pid = get_current_task();
    int next_pid = prev_pid;

    struct task_struct *prev = task_pool[prev_pid];
    struct task_struct *next = task_pool[(++next_pid) % TASK_POOL_SIZE];

    while (next == NULL || next->state != RUNNING)
    {
        next_pid = (next_pid + 1) % TASK_POOL_SIZE;
        next = task_pool[next_pid];
    }

    update_current_task(next_pid);
    context_switch(&prev->context, &next->context);

    enable_preemption();
}

void init_schedule()
{
    for (int i = 0; i < TASK_POOL_SIZE; i++)
        task_pool[i] = NULL;

    int pid = thread_create(idle_thread);
    update_current_task(pid);
}

void task_preemption()
{
    struct task_struct *current = task_pool[get_current_task()];
    if (current->preemptible && current->need_schedule)
    {
        current->quota = TASK_QUOTA;
        current->need_schedule = 0;
        sys_schedule();
    }
}
