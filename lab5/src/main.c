#include "uart.h"
#include "shell.h"
#include "printf.h"
#include "mm.h"
#include "timer.h"
#include "utils.h"
#include "entry.h"
#include "fork.h"
#include "sched.h"
#include "sys.h"
#include "cpio.h"

void foo(){
    for(int i = 0; i < 2; ++i) {
        printf("Thread id: %d %d\n", current->pid, i);
        delay(10000);
        schedule();
    }
    
    exit();
}

void fork_test()
{
    /* Test fork */
    printf("\n[fork_test]Fork Test, pid %d\n", getpid());
    int cnt = 1;
    int ret = 0;
    printf("pid: %d, cnt: %d, cnt_adress: 0x%x\n", getpid(), cnt, &cnt);
    if (fork() == 0) { // child
        printf("pid: %d, cnt: %d, cnt_adress: 0x%x\n", getpid(), cnt, &cnt);
        ++cnt;
        fork();
        while (cnt < 5) {
            printf("pid: %d, cnt: %d, cnt_adress: 0x%x\n", getpid(), cnt, &cnt);
            delay(1000000);
            ++cnt;
        }
    } else {
        printf("parent here, pid %d, child %d\n", getpid(), ret);
    }
    // dumpTasksState();
    printf("[exit] Task%d\n", getpid());
    exit();
}

int exec_argv_test(int argc, char **argv) {
    printf("\n[exec_argv_test]Argv Test, pid %d\n", getpid());
    for (int i = 0; i < argc; ++i) {
        printf("%s\n", argv[i]);
    }
    
    char *fork_argv[] = {"fork_test", 0};
    exec("fork_test", fork_argv);
    return -1;
}

void user_process(){
    /* Test syscall write */
    // call_sys_write("[user_process]User process started\n");
    
    // /* Test syscall getPID */
    // int current_pid = getpid();
    // printf("[getPID] Test syscall getpid\n");
    // printf("[getPID]Current Pid = %d\n", current_pid);

    // /* Test syscall uart_write*/
    // printf("[uart_write] Test syscall uart_write\n");
    // int size = uartwrite("[uart_write] syscall test\n", 26);
    // printf("[uart_write] How many byte written = %d\n", size);

    // /* Test syscall uart_read */
    // printf("[uart_read] Test syscall uart_read\n");
    // char uart_read_buf[20];
    // int size = uartread(uart_read_buf, 4);
    // printf("\n[uart_read] Read buf = %s, How many byte read = %d\n", uart_read_buf, size);

    // /* Test syscall malloc */
    // printf("[malloc] Test syscall malloc\n");
    // void *malloc_return = call_sys_malloc(PAGE_SIZE);
    // printf("[malloc] Allocated starting address of page = 0x%x\n", malloc_return);

    /* Test syscall exec with argument passing */
    char* argv[] = {0};
    // exec("argv_test.img", argv);
    exec("syscall.img", argv);

    /* syscall exit */
    printf("[exit] Task%d exit\n", getpid());
    exit();
}

void test_waitQueue_uart_read()
{
    printf("[uart_read] Test syscall uart_read\n");
    char uart_read_buf[20];
    int size = uartread(uart_read_buf, 3);
    printf("\n[uart_read] Read buf = %s, How many byte read = %d\n", uart_read_buf, size);
    //printf("[exit] Task%d exit\n", getpid());
    while(1);
    exit_process();
}

void kernel_process(){
    // printf("[kernel_process]Kernel process started. EL %d\r\n", get_el());
    int err = move_to_user_mode((unsigned long)&user_process);
    if (err < 0){
        printf("Error while moving process to user mode\n\r");
    } 
}


int main()
{
    // set up serial console
    uart_init();
    
    // Initialize printf
    init_printf(0, putc);

    // Initialize memory allcoator
    mm_init();

    // Turn on core timer interrupt
    core_timer_enable();
    // enable IRQ interrupt
    enable_irq();
    
    enable_uart_interrupt();

    // say hello
    // printf(init_logo);

    
    /* Test cases */
    // Requirement 1 - Implement the thread mechanism. 
    for(int i = 0; i < 3; ++i) { // N should
        int res = copy_process(PF_KTHREAD, (unsigned long)&foo, 0, 0);
        if (res < 0) {
         printf("error while starting kernel process");
         return 0;
       }
    }

    //  Requirement 2 
    int res = copy_process(PF_KTHREAD, (unsigned long)&kernel_process, 0, 0);
    if (res < 0) {
        printf("error while starting kernel process");
        return 0;
    }
    
    // Elevtive 1 - Wait Queue
    // res = copy_process(PF_KTHREAD, (unsigned long)&test_waitQueue_uart_read, 0, 0);
    // if (res < 0) {
    //     printf("error while starting kernel process");
    //     return 0;
    // }
    // int i = 0;
    while (1) {
        // printf("In kernel main()\n");
        // dumpTasksState();
        kill_zombies(); // reclaim threads marked as DEAD
        delay(100000);
        // printf("Thread %d IDLE sec %d\n", current->pid,++i);
        schedule();
    }


    
    // start shell
    if (kernel_shell_status == KERNEL_SHELL_ENABLE)
        shell_start();

    return 0;
}
