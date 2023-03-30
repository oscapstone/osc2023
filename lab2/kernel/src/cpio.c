#include "cpio.h"
#include "uart.h"
#include "shell.h"
#include "math.h"

extern void* CPIO_DEFAULT_PLACE;


/* example cpio file:
4->100 1 is a directory.
070701 008A9ACA 0000"4"1FD 000003E8000003E8000000046411C74800000000000001030000000600000000000000000000000200000000 .   //root folder
070701 008A9B91 000081B4 000003E8000003E8000000016411C7290000000F000001030000000600000000000000000000000600000000 1.txt this is 1.txt.
070701008A9ACD 000041FD 000003E8000003E8000000026411C74600000000000001030000000600000000000000000000000200000000 1 //folder
070701008A9B6C 000081B4 000003E8000003E8000000016411C7310000000E000001030000000600000000000000000000000600000000 2.txt this is 2.txt
070701008A9B77 000041FD 000003E8000003E8000000026411C74800000000000001030000000600000000000000000000000200000000 2 //folder
07070100000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000B00000000 TRAILER!!!
*/

inline unsigned long align4(unsigned long n){
	return ((n +3) & ~0x3);  // if (n%4) return ((n>>2)+1) <<2;
}

void list(){
  uart_send_string("List of files:\n");

  cpio_newc_header *blk = CPIO_DEFAULT_PLACE; 

  char *name = ((char *)blk + sizeof(cpio_newc_header)); //110
  //TRAILER!!! is the end of cpio
  while(str_cmp(name, "TRAILER!!!")!=1){
    unsigned long mode = hex_to_uint(blk->c_mode, 8); 
    unsigned long filesize = hex_to_uint(blk->c_filesize, 8); 
    unsigned long namesize = hex_to_uint(blk->c_namesize, 8);
    if (mode & (1 << 14)) uart_send_string(" <DIR> "); 
    else uart_send_string("<FILE> ");

    char print_c[20];
    uart_send_string(name);
    uart_send_string("\n");

    char *context = (char *) align4((unsigned long)name+namesize); 

    blk = (cpio_newc_header *) align4((unsigned long)context+filesize); 
    name = ((char *)blk + sizeof(cpio_newc_header)); 
  }
}


void show_cat_file(char *file_name){

  cpio_newc_header *blk = CPIO_DEFAULT_PLACE;

  char *name = ((char *)blk + sizeof(cpio_newc_header));
  while(str_cmp(name, "TRAILER!!!") != 1){
    unsigned long mode = hex_to_uint(blk->c_mode, 8);
    unsigned long filesize = hex_to_uint(blk->c_filesize, 8); 
    unsigned long namesize = hex_to_uint(blk->c_namesize, 8);
    char *context = (char *) align4((unsigned long)name+namesize);
    if (str_cmp(name, file_name)){
      if (mode & (1 << 14)){
        uart_send_string("\"");
        uart_send_string(file_name);
        uart_send_string("\" is a directory.\n");
        return;
      }
      else{
        uart_send_cat_file(context, filesize);
        uart_send_string("\n");
        return;
      }
    }
    blk = (cpio_newc_header *) align4((unsigned long)context+filesize);
    name = ((char *)blk + sizeof(cpio_newc_header)); 
  }
  uart_send_string("File \"");
  uart_send_string(file_name);
  uart_send_string("\" not found.\n");

  return;
}