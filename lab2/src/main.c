#include "uart.h"
#include "mbox.h"
#include "shell.h"
#include "string.h"
#include "malloc.h"
#include "dtb.h"
#include "cpio.h"

void init_cpio_default_place();

extern char* dtb_place;

void main(char* dtb)
{
    //dtb pointer to global (device tree)
    dtb_place = dtb;
    init_cpio_default_place(); //cpio pointer to global (file system)
    //test malloc
    char* test1 = malloc(0x18);
    memcpy(test1,"test malloc1",sizeof("test malloc1"));
    uart_printf("%s\n",test1);
    char* test2 = malloc(0x20);
    memcpy(test2,"test malloc2",sizeof("test malloc2"));
    uart_printf("%s\n",test2);
    char* test3 = malloc(0x28);
    memcpy(test3,"test malloc3",sizeof("test malloc3"));
    uart_printf("%s\n",test3);

    uart_printf("dtb : 0x%x\n",dtb);

    shell();
}

void init_cpio_default_place()
{
    traverse_device_tree(dtb_place,dtb_callback_initramfs);
}
