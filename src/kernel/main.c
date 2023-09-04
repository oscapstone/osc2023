#include <mini_uart.h>
#include <string.h>
#include <builtin.h>
#include <cpio.h>
#include <dt17.h>
#include <utils.h>
#include <timer.h>
#include <irq.h>
#include <mm.h>
#include <sched.h>
#include <kthread.h>
#include <task.h>
#include <exec.h>
#include <fsinit.h>

#define BUFSIZE 0x100
char shell_buf[BUFSIZE];
char *fdt_base; 

static void idle(void){
    while(1){
        kthread_kill_zombies();
        schedule();
    }
}

void shell_interact(void){
    while(1){
        uart_printf("# ");
        // uart_printf("initial ramdisk addr:%x\r\n", _initramfs_addr);
        unsigned int cnt = uart_recv_line(shell_buf, BUFSIZE);
        uart_printf("\r\n");
        if (!strcmp("help", shell_buf))
            _help(0);
        else if (!strcmp("hello", shell_buf)) 
            _hello();
        else if (!strcmp("reboot", shell_buf))
            _reboot();
        else if (!strcmp("hwinfo", shell_buf))
            _hwinfo();
        else if (!strcmp("ls", shell_buf))
            _ls();
        else if (!strncmp("cat", shell_buf, 3)){
            if(cnt >= 5)
                _cat(&shell_buf[4]);
            else
                uart_printf("usage: cat <filename>\r\n");
        }
        else if (!strncmp("malloc", shell_buf, 6)){
            if(cnt >= 7){
                char *string = _malloc(&shell_buf[7]);
                if(string)
                    uart_printf("[*] malloc address: %x\r\n", string);
            }
            else
                uart_printf("usage: malloc <size>\r\n");
        }
        else if (!strcmp("parsedtb",shell_buf))
            _parsedtb(fdt_base);
        else if (!strncmp("exec",shell_buf,4)){
            if(cnt >= 6 && shell_buf[4]==' ')
                _exec(&shell_buf[5]);
            else
                uart_printf("usage: exec <filename>\r\n");
        }
        else if (!strcmp("chmod_uart",shell_buf))
            _chmod_uart();
        else if (!strncmp("setTimeout",shell_buf,10)){
            if(_setTimeout(&shell_buf[10])==-1)
                uart_printf("usage: setTimeout <msg> <sec>\r\n");
        }
        else if (!strcmp("chmod_timer",shell_buf))
            timer_switch_info();
        else if (!strcmp("thread_test", shell_buf)){
            _thread_test();
        }
        else {
            _echo(shell_buf);
            if(cnt)
                uart_printf("\r\n");
        }
    }
}

void kernel_main(char *fdt){

    irq_init();

    fdt_base = fdt;
    uart_init();

    initramfs_init(fdt);
    mm_init(fdt);
    timer_init();
    task_init();
    
    scheduler_init();

    kthread_early_init();
    fs_early_init();
    kthread_init();
    

    uart_printf("[*] Kernel start running!\r\n");
    uart_printf("[*] fdt base: %x\r\n", fdt);

    // kthread_create(shell_interact);
    sched_new_user_prog("/initramfs/vfs2.img");

    enable_irqs1();
    enable_interrupt();
    
    idle();
}