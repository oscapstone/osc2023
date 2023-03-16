#include "system.h"
#include "uart.h"
#include "mbox.h"

extern char _start[]; //bootloader load kernel to here
char* _dtb;

#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024

/* For all return 0 -> success , -1 failure*/

int get_board_revision(unsigned int* board_revision)
{
    /*
        GET_BOARD_REVISION
    */
    mbox[0] = 7*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // request code
    mbox[2] = GET_BOARD_REVISION;   // tag identifier
    mbox[3] = 4;                    // value buffer size in bytes
    mbox[4] = 0;                    // request codes : b31 clear, b30-b0 reversed
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = MBOX_TAG_LAST;        // end tag
    // send the message to the GPU and receive answer
    if (mbox_call(MBOX_CH_PROP)) {
        *board_revision = mbox[5];
        return 0;
    } else {
        uart_puts("Unable to query serial!");
        *board_revision = mbox[5] = -1;
        return -1;
    }
}

int get_arm_memory_info(unsigned int* base_addr,unsigned int* size)
{
    /*
        GET arm_memory address and size
    */
    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // request code
    mbox[2] = GET_ARM_MEMORY;       // tag identifier
    mbox[3] = 8;                    // value buffer size in bytes
    mbox[4] = 0;                    // request codes : b31 clear, b30-b0 reversed
    mbox[5] = 0;                    // clear output buffer ( u32: base address in bytes )
    mbox[6] = 0;                    // clear output buffer ( u32: size in bytes )
    mbox[7] = MBOX_TAG_LAST;        // end tag
    
    // send the message to the GPU and receive answer
    if (mbox_call(MBOX_CH_PROP)) {
        *base_addr = mbox[5];
        *size = mbox[6];
        return 0;
    } else {
        uart_puts("Unable to query serial!");
        return -1;
    }
}

void set(long addr, unsigned int value) 
{
    volatile unsigned int* point = (unsigned int*)addr;
    *point = value;
}

void reboot()
{
    //disable_uart();
    reset(1); // timeout = 1/16th of a second? (whatever)
} 

void reset(int tick) 
{                                      // reboot after watchdog timer expire
    set(PM_RSTC, PM_PASSWORD | 0x20);  // full reset
    set(PM_WDOG, PM_PASSWORD | tick);  // number of watchdog tick
    while(1);                          // wati for clock
}

void cancel_reset() 
{
    set(PM_RSTC, PM_PASSWORD | 0);  // full reset
    set(PM_WDOG, PM_PASSWORD | 0);  // number of watchdog tick
}


//編譯器不要優化這段
#pragma GCC push_options
#pragma GCC optimize ("O0")

void load_kernel()
{
    // prevent dtb been rewrited by kernel 
    char* temp_dtb = _dtb;
    char c;
    unsigned long long kernel_size=0;
    char* kernel_start = (char*) (&_start);

    uart_puts("kernel size:");
    for(int i=0;i<8;i++)  //protocol : use little endian to get kernel size
    {
        c = uart_getc_pure();
        kernel_size += c<<(i*8);
    }


    for(int i=0;i<kernel_size;i++)
    {
        c = uart_getc_pure();
        kernel_start[i]=c;
    }

    uart_puts("susccess get kernel");
    uart_puts("change ip to kernel...");

    //這邊會被編譯器優話變成直接來個相對位置，導致跑到relocate 的0x70000
    ((void (*)(char*))kernel_start)(temp_dtb);   // dtb address into x0 to kernel

}
#pragma GCC pop_options