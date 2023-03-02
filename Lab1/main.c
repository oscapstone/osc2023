#include "uart.h"
#include "io.h"
#include "mailbox.h"
#include "reboot.h"

int strcmp(char *a, char *b){

    char *p1=a;
    char *p2=b;
    while(1){
        char c1 = *p1;
        p1++;
        char c2 = *p2;
        p2++;

        if(c1==c2){
            if(c1 == '\0') return 1;
            else continue;
        }
        else return 0;

    }



}



void readcmd(char *x){
    

    char input_char;
    x[0]=0;
    int input_index = 0;
    // uart_puts("readcmd start\n");
    while( (input_char = read_c()) != '\n'){
        x[input_index] = input_char ;
        input_index += 1;
        print_char(input_char);
    }

    // print_char('\n');
    x[input_index]=0;

}



void shell(){
    
    print_string("\nAnonymousELF@Rpi3B+ >>> ");

    char command[256];
    readcmd(command);
    

    if(strcmp(command,"help")){
        print_string("\nhelp       : print this help menu\n");
        print_string("hello      : print Hello World!\n");
        print_string("mailbox    : print Hardware Information\n");
        print_string("reboot     : reboot the device");
    }
    else if(strcmp(command,"hello")){
        print_string("\nHello World!");
    }else if(strcmp(command,"mailbox")){
        print_string("\nMailbox info :\n");
        get_board_revision();
        get_memory_info();
    }
    else if(strcmp(command,"reboot")){
        print_string("\nRebooting ...\n");
        reset(200);

    }else{
        print_string("\nCommand not found");
    }


}


int main()
{
    // set up serial console
    uart_init();
    uart_puts("\nWelcome to AnonymousELF's shell");
    while(1) {
        shell();
    }

    return 0;
}