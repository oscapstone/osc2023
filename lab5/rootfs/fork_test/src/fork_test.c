#include "printf.h"
#include "sys.h"
#include "uart.h"

void delay(int cycles){
    while(cycles != 0) cycles--;
}

int main(void) {
    init_printf(0, putc);
    
    // // printf("\n[fork_test]Fork Test, pid %d\n", getpid());
    // uartwrite("\n[fork_test]Fork Test, pid", 27);

    // int cnt = 1;
    // int ret = 0;
    // // printf("pid: %d, cnt: %d, cnt_adress: 0x%x\n", getpid(), cnt, &cnt);
    // uartwrite("(Outer) pid: %d, cnt: %d, cnt_adress: 0x%x\n", 44);

    // if ((ret = fork()) == 0) { // child
    //     uartwrite("(Inner 1) pid: %d, cnt: %d, cnt_adress: 0x%x\n", 46);
    //     ++cnt;
    //     fork();
    //     while (cnt < 5) {
    //         uartwrite("(Inner 2) pid: %d, cnt: %d, cnt_adress: 0x%x\n",46);
    //         for(int i = 0;i < 100000;i++);
    //             ++cnt;
    //     }
    // } else {
    //     uartwrite("parent here, pid %d, child %d\n",31);
    // }
    // // dumpTasksState();
    // uartwrite("[exit] Task%d\n", 15);
    // exit();
    // return 0;

    printf("\nFork Test, pid %d\n", getpid());
    int cnt = 1;
    int ret = 0;
    if ((ret = fork()) == 0) { // child
        long long cur_sp;
        asm volatile("mov %0, sp" : "=r"(cur_sp));
        printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
        ++cnt;

        if ((ret = fork()) != 0){
            asm volatile("mov %0, sp" : "=r"(cur_sp));
            printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
        }
        else{
            while (cnt < 5) {
                asm volatile("mov %0, sp" : "=r"(cur_sp));
                printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
                delay(1000000);
                ++cnt;
            }
        }
        exit();
    }
    else {
        printf("parent here, pid %d, child %d\n", getpid(), ret);
    }
}