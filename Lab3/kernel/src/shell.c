#include "shell.h"
#include "string.h"
#include "uart.h"
#include "dtb.h"
#include "cpio.h"
#include "mailbox.h"
#include "time.h"
#include "uart_boot.h"



void welcome_msg() {
        uart_puts(
        "                      /|      __\n"
        "*             +      / |   ,-~ /             +\n"
        "     .              Y :|  //  /                .         *\n"
        "         .          | jj /( .^     *\n"
        "               *    >-'\"'-v'              .        *        . \n"
        "*                  /       Y\n"
        "   .     .        jo  o    |     .            +\n"
        "                 ( '~'     j )                    +     .\n"
        "      +           >._-' _./         +\n"
        "               /| ;-'~ _  l\n"
        "  .           / l/ ,-'~    \\     +\n"
        "              \\\\//\\/      .- \\\n"
        "       +       Y        /    Y\n"
        "               l       I     !\n"
        "               ]\\      _\\    /\\\n"
        "              (\" ~----( ~   Y.  )\n"
        "          ~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
        " Hey, welcome to Richard's OS! Hope you like it here.\n\n"
        "-------------------------------------------------------\n"
        "=                      kernel                         =\n"
        "-------------------------------------------------------\n");    
}



void shell_start () {

    int buffer_counter = 0;
    char input_char;
    char buffer[MAX_BUFFER_LEN];
    enum SPECIAL_CHARACTER input_parse;

    strset (buffer, 0, MAX_BUFFER_LEN);   

    // new line head
    // uart_puts("# ");
    uart_puts("richard");
    uart_puts("@");
    uart_puts("raspberry");
	uart_puts(" $ ");

    // read input
    while(1){

        input_char = uart_getc();
        input_parse = parse ( input_char );
        command_controller ( input_parse, input_char, buffer, &buffer_counter);

    }
}

enum SPECIAL_CHARACTER parse ( char c ){

    if ( !(c < 128 && c >= 0) )
        return UNKNOWN;

    if ( c == BACK_SPACE )
        return BACK_SPACE;
    
    if ( c == DELETE )
        return DELETE;

    else if ( c == LINE_FEED || c == CARRIAGE_RETURN )
        return NEW_LINE;

    else
        return REGULAR_INPUT;    
}

void command_controller ( enum SPECIAL_CHARACTER input_parse, char c, char buffer[], int * counter ){

    /* args */
    char *args = buffer;  // 修改1：去掉&，args指向buffer数组的首地址
    int arg_idx = 0;
    for (; buffer[arg_idx] != '\0'; ++arg_idx) {
        if (buffer[arg_idx] == ' ') {
            buffer[arg_idx] = '\0';
            args = buffer + arg_idx + 1;
            break;
        }
    }
    /* 修改2：修正args指向字符串的位置 */
    if (arg_idx > 0) {
        args = buffer + arg_idx + 1;
    }
    
    if ( input_parse == UNKNOWN ) return;
    
    // Special key
    if ( input_parse == BACK_SPACE){
  
        if (  (*counter) > 0 ){

            uart_send('\b');
            uart_send(' ');
            uart_send('\b');
            (*counter) --;
        }
    }
    if ( input_parse == DELETE ){

        if (  (*counter) > 0 ){

            uart_send('\b');
            uart_send(' ');
            uart_send('\b');
            (*counter) --;
        }
        


    }
    else if ( input_parse == NEW_LINE ){

        uart_send(c);
        if ( (*counter) == MAX_BUFFER_LEN ) {

            input_buffer_overflow_message(buffer);
        }
        else {

            buffer[(*counter)] = '\0';
            /* debug print*/
            /*
            uart_puts("\n");
            uart_puts("buffer is ");
            uart_puts(buffer);
            uart_puts("\n");
            uart_puts("args is ");
            uart_puts(args);
            uart_puts("\n");
            */

            if      ( !strcmp(buffer, "help"        ) ) command_help();
            else if ( !strcmp(buffer, "hello"       ) ) command_hello();
            else if ( !strcmp(buffer, "reboot"      ) ) command_reboot();
            else if ( !strcmp(buffer, "info"        ) ) command_info();
            else if ( !strcmp(buffer, "clear"       ) ) command_clear();
            else if ( !strcmp(buffer, "dtb"         ) ) command_dtb();
            else if ( !strcmp(buffer, "ls"          ) ) command_ls();                        
            else if ( !strcmp(buffer, "cat"         ) ) command_cat(args);                              
            else                                        command_not_found(buffer);

        }
            
        (*counter) = 0;
        strset (buffer, 0, MAX_BUFFER_LEN); 

        // new line head;
        uart_puts("richard");
        uart_puts("@");
        uart_puts("raspberry");
	    uart_puts(" $ ");
    }
    else if ( input_parse == REGULAR_INPUT ){

        uart_send(c);
        if ( *counter < MAX_BUFFER_LEN){

            buffer[*counter] = c;
            (*counter) ++;
        }
    }
}

void command_not_found (char * s) {

    uart_puts("Err: command ");
    // uart_puts(s);
    uart_puts(" not found, try <help>\n");

}

void input_buffer_overflow_message ( char cmd[] ){

    uart_puts("Follow command: \"");
    uart_puts(cmd);
    uart_puts("\"... is too long to process.\n");
    uart_puts("The maximum length of input is 64.");

}

extern void* CPIO_DEFAULT_PLACE;
void command_help (){

    /* Lab1 Feature */
    uart_puts("help\t:  Print this help menu.\n");
    uart_puts("hello\t:  Print \"Hello World!\".\n");
    uart_puts("info\t:  Print \"Board Revision\".\n");
    uart_puts("reboot\t:  Reboot the device.\n");
    
    /* Lab2 Feature */
    uart_puts("ls\t:  List all file.\n");
    uart_puts("cat\t:  Cat the file.\n");
    uart_puts("dtb\t:  Show device tree.\n");


}

void command_hello (){

    uart_puts("Hello World!\n");

}



void command_reboot (){

    uart_puts("Start Rebooting...\n");
    *PM_WDOG = PM_PASSWORD | 0x20;
    *PM_RSTC = PM_PASSWORD | 100;
	while(1);

}

void command_info()
{
    char str[20];  
    uint32_t board_revision = mbox_get_board_revision ();
    uint32_t arm_mem_base_addr;
    uint32_t arm_mem_size;
    mbox_get_arm_memory_info(&arm_mem_base_addr, &arm_mem_size);
    
    uart_puts("Board Revision: ");
    if ( board_revision ){

        itohex_str(board_revision, sizeof(uint32_t), str);
        uart_puts(str);
        uart_puts("\n");

    }
    
    else{

        uart_puts("Unable to query serial!\n");

    }

    uart_puts("ARM memory base address: ");
    if ( arm_mem_base_addr ){

        itohex_str(arm_mem_base_addr, sizeof(uint32_t), str);
        uart_puts(str);
        uart_puts("\n");

    }

    else{

        uart_puts("0x00000000\n");

    }

    uart_puts("ARM memory size: ");
    if ( arm_mem_size ){

        itohex_str( arm_mem_size, sizeof(uint32_t), str);
        uart_puts(str);
        uart_puts("\n");

    }

    else{

        uart_puts("0x00000000\n");
    }
    
}



void command_clear (){

    uart_puts("\033[2J\033[H");

}





void command_ls(){

    char* c_filepath;
    char* c_filedata;
    unsigned int c_filesize;
    struct cpio_newc_header *header_ptr = CPIO_DEFAULT_PLACE;

    while(header_ptr!=0)
    {
        int error = cpio_newc_parse_header(header_ptr, &c_filepath, &c_filesize, &c_filedata, &header_ptr);
        //if parse header error
        if(error)
        {
            uart_puts("cpio parse error");
            break;
        }

        //if this is not TRAILER!!! (last of file)
        if(header_ptr!=0) {
            uart_puts(c_filepath);
            uart_puts("\n");
            
        };
    }
}


void command_cat(char *args){

    char* c_filepath;
    char* c_filedata;
    unsigned int c_filesize;
    struct cpio_newc_header *header_ptr = CPIO_DEFAULT_PLACE;

    while(header_ptr!=0){
        int error = cpio_newc_parse_header(header_ptr, &c_filepath, &c_filesize, &c_filedata, &header_ptr);
        //if parse header error
        if(error){
            uart_puts("cpio parse error");
            break;
        }

        if(strcmp(c_filepath, args)==0){
            uart_puts(c_filedata);
            uart_puts("\n");
            break;
        }

        //if this is TRAILER!!! (last of file)
        if(header_ptr==0) {
            uart_puts("cat:");
            uart_puts(args);
            uart_puts("No such file or directory\n");
        }
    }
}




void *base = (void *) DT_ADDR;
void command_dtb(){
    traverse_device_tree( base, dtb_callback_show_tree);
}