#include "shell.h"
#include "uart.h"
#include "my_str.h"
#include "reboot.h"

extern char* _dtb;
extern char _start[];

void shell_init(){
	uart_init();
    uart_flush();
    uart_printf("\nWelcome! OSC2023 #bootloader\n");
}
void cli_cmd_clear(char* buffer, int length)
{
    for(int i=0; i<length; i++)
    {
        buffer[i] = '\0';
    }
};

void shell_input(char *buffer){
    uart_printf("\t#");
    char c='\0';
    int idx = 0;
    while(1)
    {
        if ( idx >= CMD_MAX_LEN ) break;

        c = uart_read();
        if ( c == '\n')
        {
            uart_printf("\r\n");
            break;
        }
        buffer[idx++] = c;
        uart_write(c);
    }
}
void loadimg(){
    char* bak_dtb = _dtb; // give new address avoid overlap
    char *kernel_start = (char *)&_start;//0x80000
    char b;
    // uart_printf("%x %x\n", bak_dtb, &bak_dtb);
    uart_printf("Send image via UART now!\n");

    // big endian
    int img_size = 0, i;
    for (i = 0; i < 4; i++) {
        img_size <<= 8;
        img_size |= (int)uart_read_raw();
    }

    // big endian
    int img_checksum = 0;
    for (i = 0; i < 4; i++) {
        img_checksum <<= 8;
        img_checksum |= (int)uart_read_raw();
    }    

    for (i = 0; i < img_size; i++) {
        b = uart_read_raw();
        *(kernel_start + i) = b;
        img_checksum -= (int)b;
    }
    
    if (img_checksum != 0) {
        uart_printf("Failed!");
    }
    else {
        // uart_printf("%x %x\n", bak_dtb, &bak_dtb);
        uart_printf("Done\n");
        // function pointer
        void (*start_kernel)(char *) = (void *)&_start;
        
        start_kernel(bak_dtb);
    }    
}

void shell_controller(char *buffer){ 
    
    if (!buffer) return;

    char* cmd = buffer;
    
    if (strcmp(cmd, "") == 0){
        return;
    }
    else if (strcmp(cmd, "help") == 0){
        uart_printf("help\t: print the help menu\n");
        uart_printf("hello\t: print Hello World!\n");
        uart_printf("loadimg\t: load image\n");
        uart_printf("reboot\t: reboot the device\n");        
    }
    else if (strcmp(cmd, "hello") == 0){
        uart_printf("Hello World!\n");
    }
    else if (strcmp(cmd, "loadimg") == 0){
        loadimg();
    }
    else if (strcmp(cmd, "reboot") == 0){
        uart_printf("Rebooting..\n");
        reset(5);
        while(1);
    }
    else{
        uart_printf("Commont not found!\n");
    }
}