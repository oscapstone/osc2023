#include "uart.h"
#include "io.h"
#include "mailbox.h"
#include "reboot.h"
#include "cpio.h"
#include "mem.h"
#include "string.h"
#include "devtree.h"

#define MAX_LEN 1024


uint64_t initrd_addr;

int readcmd(char *buf, int len);

void cat() {
    static char input[MAX_LEN]={0}, fname[MAX_LEN]={0};



    print_string("\nType In Filename: ");

    readcmd(input,MAX_LEN);
    print_char('\n');
    // print_string(input);

    // return;
    struct cpio_newc_header *p_header = cpio_first(initrd_addr);
    // print_string(p_header);
    while (p_header != 0) {
        cpio_filename(p_header, fname, MAX_LEN);
        if (strcmp(fname, input)) {
            uint32_t off = 0;
            uint32_t len = 0;
            while ((len = cpio_read(p_header, off, input, MAX_LEN-1)) != 0) {
            input[len] = 0;
            off += len;
            print_string(input);
            }
            
            return;
        }
        p_header = cpio_nextfile(p_header);
    }
    print_string("File does not exist!\n");
}

void initramfs_callback( char* node_name,  char *prop_name, void* value, uint32_t size) {
  
    // initrd_addr = (uint32_t)0x08200000;
    if (strcmp(node_name, "chosen") && strcmp(prop_name, "linux,initrd-start")) {
    //   print_string("\nha\n");
        uint32_t initaddr = fdt32_to_cpu(*((uint32_t*)value));
    //   print_h(initaddr);
        initrd_addr = initaddr;//0x2000000;
        print_string("Init addr ");
        print_h(initrd_addr);
        print_char('\n');
        print_string("Size is ");
        print_h(size);
        print_char('\n');

    }
}






void shell(){
    
    print_string("\nAnonymousELF@Rpi3B+ >>> ");

    char command[256];
    readcmd(command,256);
    static char buf[MAX_LEN];


    if(strcmp(command,"help")){
        print_string("\nhelp       : print this help menu\n");
        print_string("hello      : print Hello World!\n");
        print_string("mailbox    : print Hardware Information\n");
        print_string("ls         : list files existed in this folder\n");
        print_string("cat        : print specific file content\n");
        print_string("test malloc: test malloc function\n");
        print_string("reboot     : reboot the device");
    
    }else if(strcmp(command,"hello")){
        print_string("\nHello World!");
    
    }else if(strcmp(command,"mailbox")){
        print_string("\nMailbox info :\n");
        get_board_revision();
        get_memory_info();
    
    }else if(strcmp(command,"reboot")){
        print_string("\nRebooting ...\n");
        reset(200);

    }else if(strcmp(command,"ls")){
        print_char('\n');

        struct cpio_newc_header *p_header = cpio_first(initrd_addr);
        // print_h(initrd_addr);
        while (p_header != 0) {
            // print_string("aaaaaaaaa\n");
            cpio_filename(p_header, buf, MAX_LEN);
            print_string(buf);
            print_char('\n');
            p_header = cpio_nextfile(p_header);
        }

    
    }else if(strcmp(command,"cat")){
        cat();
    
    }else if(strcmp(command,"test malloc")){
        print_string("\nTest str1 = malloc(7) ; str1 = \"Success\"\n");
        char *str1 = (char *)simple_malloc(7);
        strncpy(str1,"Success",7);
        str1[7]=0;
        print_string("Answer str1 = ");
        print_string(str1);
        print_char('\n');
        print_string("Answer str1 address = ");
        print_h(str1);
        print_char('\n');

        print_string("\nTest str2 = malloc(30) ; str2 = \"Longer String Success Too\"\n");
        char *str2 = simple_malloc(30);
        strncpy(str2,"Longer String Success Too",30);
        str2[30]=0;
        print_string("Answer str2 = ");
        print_string(str2);
        print_char('\n');
        print_string("Answer str2 address = ");
        print_h(str2);
        print_char('\n');
    }
    else{
        print_string("\nCommand not found");
    
    }


}

int main( )
{
    
    mini_uart_init();
    mem_init();
    
    print_string("\nWelcome to AnonymousELF's shell");

    uint32_t* t = 0x50000;
    print_char('\n');
    print_string("Dtb address contain is \n");
    print_h(*t);
    print_char('\n');
    fdt_traverse(initramfs_callback);

    while(1) {
        shell();
    }

    return 0;
}


int readcmd(char *buf, int len){
    


    char c;
    int i;
    for (i = 0; i < len; i++) {
        c = mini_uart_recv();
        if (c == 127) { i--; continue; }
        print_char(c);
        // print_num((int)c);
        if (c == '\r') {
        c = '\n';
        print_char('\n');
        break;
        }
        buf[i] = c;
    }
    buf[i] = '\0';
    return i;



}



