#include "printf.h"
#include "uart.h"
#include "syscall.h"

void main(void)
{
    init_printf(0, putc);
    int cnt = 1;
    int ret = 0;
    if ((ret = fork()) == 0)
    { // child
        long long cur_sp;
        asm volatile("mov %0, sp"
                     : "=r"(cur_sp));
        printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
        ++cnt;

        if ((ret = fork()) != 0)
        {
            asm volatile("mov %0, sp"
                         : "=r"(cur_sp));
            printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
        }
        else
        {
            while (cnt < 5)
            {
                asm volatile("mov %0, sp"
                             : "=r"(cur_sp));
                printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
                ++cnt;
            }
        }
        exit();
    }
    else
    {
        printf("parent here, pid %d, child %d\n", getpid(), ret);
        exit();
    }
}
