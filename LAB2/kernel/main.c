#include "mini_uart.h"
#include "dtb.h"
#include "shell.h"

extern void *_dtb_ptr; // 哪裡來的? kenenl的start.s定義的

void kernel_main(void)
{
    // uart_init();
    uart_send_string("Hello, world!\n");
    // 以下是猜測?????
    // 當轉來執行kernel.img,用function pointer的方式傳遞_dtb參數過來
    // 這時_dtb_ptr在kernel's start.s中透過x0拿到的就變成是_dtb的位置(而非dtb的位置)
    // 所以_dtb_ptr是指向_dtb的指標，而_dtb是指向dtb的指標，
    fdt_traverse(get_initramfs_addr, _dtb_ptr); // 先print出當前dtb && cpio的最新擺放位置
    shell();
}