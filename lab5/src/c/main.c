#include "shell.h"
#include "uart.h"
#include "printf.h"
#include "mm.h"
#include "timer.h"
#include "schedule.h"
#include "syscall.h"
#include "sysregs.h"

void foo()
{
    for (int i = 0; i < 5; i++)
    {
        delay(10000000);
        printf("[FOO] pid: %d %d\n", getpid(), i);
    }
    exit();
}

void user_test()
{
    const char *argv[] = {0x0};
    // exec("fork_test.img", argv);
    exec("syscall.img", NULL);
}

int main()
{
    // printf("Hello World!\n\n");
    // shell_start();

    init_uart();
    init_printf(0, putc);
    init_memory();
    init_schedule();
    init_timer();

    for (int i = 0; i < 5; i++)
        thread_create(foo);
    thread_create(user_test);

    unsigned int current_pid = get_current_task();
    struct task_struct *current_task = task_pool[current_pid];
    start_context(&current_task->context);

    return 0;
}