#include "uart.h"

void main()
{
    int size=17038;
    char *kernel=(char*)0x80000;

    // set up serial console
    uart_init();

    // read the kernel
    while(size--)
    {
	*kernel++ = uart_getc();
    }

    // restore arguments and jump to the new kernel.
    asm volatile (
        "mov x0, x27;"
        // we must force an absolute address to branch to
        "mov x30, #0x80000;"
	"ret"
    );
    return;
}
