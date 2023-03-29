#include "shell.h"
#include "string.h"
#include "uart.h"
#include "dtb.h"
#include "cpio.h"
#include "mailbox.h"
#include "uart_boot.h"
#include "malloc.h"
#include "vt.h"
#include "timer.h"
#include "command.h"


void welcome_msg() {

        uart_async_puts(
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
    // uart_async_puts("# ");
    uart_async_puts("richard");
    uart_async_puts("@");
    uart_async_puts("raspberry");
	uart_async_puts(" $ ");

    // read input
    while(1){

        input_char = uart_async_getc();
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

            uart_async_putc('\b');
            uart_async_putc(' ');
            uart_async_putc('\b');
            (*counter) --;
        }
    }
    if ( input_parse == DELETE ){

        if (  (*counter) > 0 ){

            uart_async_putc('\b');
            uart_async_putc(' ');
            uart_async_putc('\b');
            (*counter) --;
        }
        


    }
    else if ( input_parse == NEW_LINE ){

        //uart_async_putc(c);
        if ( (*counter) == MAX_BUFFER_LEN ) {

            input_buffer_overflow_message(buffer);
        }
        else {

            buffer[(*counter)] = '\0';
            /* debug print*/
            /*
            uart_async_puts("\n");
            uart_async_puts("buffer is ");
            uart_async_puts(buffer);
            uart_async_puts("\n");
            uart_async_puts("args is ");
            uart_async_puts(args);
            uart_async_puts("\n");
            */

            if      ( !strcmp(buffer, "help"        ) ) command_help();
            else if ( !strcmp(buffer, "hello"       ) ) command_hello();
            else if ( !strcmp(buffer, "reboot"      ) ) command_reboot();
            else if ( !strcmp(buffer, "info"        ) ) command_info();
            else if ( !strcmp(buffer, "clear"       ) ) command_clear();
            else if ( !strcmp(buffer, "dtb"         ) ) command_dtb();
            else if ( !strcmp(buffer, "ls"          ) ) command_ls();                        
            else if ( !strcmp(buffer, "cat"         ) ) command_cat(args);     
            else if ( !strcmp(buffer, "exec"        ) ) command_exec(args);    
            else if ( !strcmp(buffer, "el"          ) ) command_el();
            else if ( !strcmp(buffer, "async"       ) ) uart_async_puts("async puts test success!!!\n");  
            else if ( !strcmp(buffer, "setTimeout"  ) ) command_setTimeout(args);
            else if ( !strcmp(buffer, "set2s"       ) ) command_set2sAlert();
            else                                        command_not_found(buffer);

        }
            
        (*counter) = 0;
        strset (buffer, 0, MAX_BUFFER_LEN); 

        // new line head;
        uart_async_puts("richard");
        uart_async_puts("@");
        uart_async_puts("raspberry");
	    uart_async_puts(" $ ");
    }
    else if ( input_parse == REGULAR_INPUT ){

        //uart_async_putc(c);
        if ( *counter < MAX_BUFFER_LEN){

            buffer[*counter] = c;
            (*counter) ++;
        }
    }
}


void input_buffer_overflow_message ( char cmd[] ){

    uart_async_puts("Follow command: \"");
    uart_async_puts(cmd);
    uart_async_puts("\"... is too long to process.\n");
    uart_async_puts("The maximum length of input is 64.");

}
