#include "shell.h"
#include "string.h"
#include "command.h"
#include "uart.h"
#include "dtb_parser.h"



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





void shell_start () 
{
    int buffer_counter = 0;
    char input_char;
    char buffer[MAX_BUFFER_LEN];
    enum SPECIAL_CHARACTER input_parse;

    strset (buffer, 0, MAX_BUFFER_LEN);   

    

    // new line head
    //uart_puts("# ");
    uart_puts("richard");
    uart_puts("@");
    uart_puts("raspberry");
	uart_puts(" $ ");

    // read input
    while(1)
    {
        input_char = uart_getc();

        

        input_parse = parse ( input_char );

        command_controller ( input_parse, input_char, buffer, &buffer_counter);
    }
}

enum SPECIAL_CHARACTER parse ( char c )
{
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

void command_controller ( enum SPECIAL_CHARACTER input_parse, char c, char buffer[], int * counter )
{   
   
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
        args = buffer + arg_idx + 1;}
    
    
    

    if ( input_parse == UNKNOWN )
        return;
    
    // Special key
    if ( input_parse == BACK_SPACE)
    {
  
        if (  (*counter) > 0 )
        {
            uart_send('\b');
            uart_send(' ');
            uart_send('\b');
            (*counter) --;
        }
    }
    if ( input_parse == DELETE )
    {   
        if (  (*counter) > 0 )
        {
            uart_send('\b');
            uart_send(' ');
            uart_send('\b');
            (*counter) --;
        }
        




    }
    else if ( input_parse == NEW_LINE )
    {
        uart_send(c);
        if ( (*counter) == MAX_BUFFER_LEN ) 
        {
            input_buffer_overflow_message(buffer);
        }
        else 
        {
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
            else if ( !strcmp(buffer, "timestamp"   ) ) command_timestamp();
            else if ( !strcmp(buffer, "reboot"      ) ) command_reboot();
            else if ( !strcmp(buffer, "info"        ) ) command_info();
            else if ( !strcmp(buffer, "clear"       ) ) command_clear();
            else if ( !strcmp(buffer, "ls"          ) ) command_cpio_list();
	        else if ( !strcmp(buffer, "cat"         ) ) command_cpio_cat(args);
            else if ( !strcmp(buffer, "dtb_ls"      ) ) command_dt_info();
	        else if ( !strcmp(buffer, "dtb_cat"     ) ) command_parse_dt();
            else if ( !strcmp(buffer, "dtb"         ) ) command_dtb();
	        else if ( !strcmp(buffer, "cpiotest"    ) ) cpio_test();                           
                                        
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
    else if ( input_parse == REGULAR_INPUT )
    {
        uart_send(c);
        if ( *counter < MAX_BUFFER_LEN)
        {
            buffer[*counter] = c;
            (*counter) ++;
        }
    }
}