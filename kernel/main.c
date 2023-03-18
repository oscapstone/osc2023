#include "uart.h"
#include "shell.h"
#include "hardware_info.h"
#include "dtb.h"
#include "utils.h"
#include "initramfs.h"
#include "note.h"
extern char *_dtb_ptr;
extern char _stext;
extern char _etext;
int main(void)
{
    uart_init();
    uart_flush();
    uart_write_string("Hello kernel!\n");
    uart_write_string("device tree address: ");
    // dump_hex(&_dtb_ptr, 8);
    uart_write_no_hex((unsigned int)_dtb_ptr);
    uart_write_string("\n");
    int fdt_ret = fdt_init(&_fdt, (unsigned int)_dtb_ptr);
    if (fdt_ret) {
        uart_write_string("fail on fdt_init!\n");
    } else {
        uart_write_string("fdt inited!\n");
        if (_fdt.fdt_traverse(&_fdt, initramfs_fdt_cb, NULL)) {
            uart_write_string("fdt traverse fail!\n");
        }
        uart_write_string("found cpio address!\n");
    }
    dump_hex(&cpio_addr, 8);
    init_initramfs(&_initramfs);
    init_note();
    print_hw_info();
    uart_write_string("text section starts at: ");
    uart_write_no_hex((unsigned long long)&_stext);
    uart_write_string("\n");
    uart_write_string("text section ends at: ");
    uart_write_no_hex((unsigned long long)&_etext);
    uart_write_string("\n");
    shell_main();
    return 0;
}
