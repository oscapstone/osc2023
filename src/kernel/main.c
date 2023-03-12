#include <mini_uart.h>
#include <string.h>
#include <builtin.h>
#include <cpio.h>

#define BUFSIZE 0x100
char shell_buf[BUFSIZE];
char* fdt_base;
uint64 _initramfs_addr;

void shell_interact(void){
    uart_printf("# ");
    unsigned int cnt = uart_recv_line(shell_buf, BUFSIZE);
    uart_printf("\r\n");
    // uart_printf("return : %d. \r\n", strncmp("cat", shell_buf, 3));
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
        // uart_printf("in the cat\r\n");
        if(cnt >= 5)
            _cat(_initramfs_addr, &shell_buf[4]);
        else
            uart_printf("usage: cat <filename>\r\n");
    }
    else {
        _echo(shell_buf);
        if(cnt)
            uart_printf("\r\n");
    }
}

void kernel_main(char *fdt){

    fdt_base = fdt;
    _initramfs_addr = 0x8000000;
    uart_init();
    uart_printf("[*] fdt base: %x\r\n", fdt_base);
    uart_printf("[*] Kernel start running!\r\n");

    while(1)
        shell_interact();
}