#include <mini_uart.h>
#include <string.h>
#include <builtin.h>
#include <cpio.h>
#include <dt17.h>

#define BUFSIZE 0x100
char shell_buf[BUFSIZE];
char *fdt_base; 

void shell_interact(void){
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
        _ls(_initramfs_addr);
    else if (!strncmp("cat", shell_buf, 3)){
        if(cnt >= 5)
            _cat(_initramfs_addr, &shell_buf[4]);
        else
            uart_printf("usage: cat <filename>\r\n");
    }
    else if (!strncmp("malloc", shell_buf, 6)){
        if(cnt >= 7){
            char* string = _malloc(&shell_buf[7]);
            if(string)
                uart_printf("[*] malloc address: %x\r\n", string);
        }
        else
            uart_printf("usage: malloc <size>\r\n");
    }
    else if (!strcmp("parsedtb",shell_buf))
        _parsedtb(fdt_base);
    else {
        _echo(shell_buf);
        if(cnt)
            uart_printf("\r\n");
    }
}

void kernel_main(char *fdt){

    fdt_base = fdt;
    uart_init();
    uart_printf("[*] Kernel start running!\r\n");
    uart_printf("[*] fdt base: %x\r\n", fdt);

    initramfs_init(fdt);

    while(1)
        shell_interact();
}