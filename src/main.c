#include "uart.h"
#include "command.h"
#include "mailbox.h"
#include "reboot.h"
#define RECV_LEN 32
void main(){
  // set up serial console
    uart_init();
    
    // say hello
    uart_puts("\r\n\t\tWelcome NYCU OSC 2023!\r\n");
    uart_puts("Hardware Info:\r\n\t");
    unsigned int revision, base, size;
    get_board_revision(&revision);
    uart_puts("Board Revision: ");
    uart_hex(revision);
    uart_puts("\r\n\tARM MEMORY BASE: ");
    get_arm_memory(&base, &size);
    uart_hex(base);
    uart_puts("\r\n\tARM MEMORY SIZE: ");
    uart_hex(size);
    uart_puts("\r\n");
    char recv[32];
    int pos = 0;
    char recv_char;
    while(1) {
        recv_char = uart_getc();
        uart_send(recv_char);
        if(recv_char == '\n'){
          

          int cmd = CMD_HELP;
          char* str_help = "help";
          char* str_hello = "hello";
          char* str_reboot = "reboot";

          // parse the command 
          for(int i = 0; i <= pos; i++){
            if(*(recv + i) != *(str_help + i)){
              cmd = CMD_INVAILD;
              break;
            }
          }
          if(cmd == CMD_HELP){
            uart_puts("\r\n>>> help\nhelp   : print this help menu\r\nhello   : print Hellp World!\nreboot    : reboot the device\r\n");
          }
          else{
            cmd = CMD_HELLO;
            for(int i = 0; i <= pos; i++){
              if(*(recv + i) != *(str_hello + i)){
                cmd = CMD_INVAILD;
                break;
              }
            }
            if(cmd == CMD_HELLO){
              uart_puts("\r\n>>> HELLO WORLD!\r\n");
            }
            else{
              cmd = CMD_REBOOT;
              for(int i = 0; i <= pos; i++){
                if(*(recv + i) != *(str_reboot + i)){
                  cmd = CMD_INVAILD;
                  break;
                }
              }
              if(cmd == CMD_REBOOT){
                reset(10);
              }
              else{
                uart_puts("\r\n>>> INVALID COMMAND\r\n");
              }
            }
          }
        
          // reset the position of the revb buffer
          pos = 0;
          // clean the contetn in the recv buffer
          for(int i = 0; i < RECV_LEN; i++){
            recv[i] = 0;
          }
        }
        else{
          recv[pos] = recv_char;
          if(pos == 31) pos = 0;
          else pos++;
        }
    }
}