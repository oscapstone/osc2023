#include "uart.h"
#include "command.h"
#include "mailbox.h"
#include "reboot.h"
#include "string.h"
#include "allocate.h"
#include "fdt.h"
#define RECV_LEN 100
char recv_buf[RECV_LEN] = {0};

// total 110 bytes 
struct cpio_newc_header {
		   char	   c_magic[6];
		   char	   c_ino[8];
		   char	   c_mode[8];
		   char	   c_uid[8];
		   char	   c_gid[8];
		   char	   c_nlink[8];
		   char	   c_mtime[8];
		   char	   c_filesize[8];
		   char	   c_devmajor[8];
		   char	   c_devminor[8];
		   char	   c_rdevmajor[8];
		   char	   c_rdevminor[8];
		   char	   c_namesize[8];
		   char	   c_check[8];
	   };

int _8byte_hexadecimal_to_int(char* msb){
  int ret = 0;
  for(int i = 0; i < 8; i++){
    int temp = *(msb + i);
    if(temp >= 'a'){
      temp = temp - 'a' + 10;
    }
    else{
      temp -= '0';
    }
    ret *= 16;
    ret += temp;
  }
  return ret;
}

void ls_cmd(){
  struct cpio_newc_header *pos = (struct cpio_newc_header*)CPIO_BASE;
  char *current = (char*) CPIO_BASE;
  while(1){
    pos = (struct cpio_newc_header*)current;
    int namesize = _8byte_hexadecimal_to_int(pos->c_namesize);
    int filesize = _8byte_hexadecimal_to_int(pos->c_filesize);
    current += sizeof(struct cpio_newc_header);
    if(strcmp(current, "TRAILER!!!") == 0) break;
    uart_puts(current);
    uart_puts("\r\n");
    // total size of the fixed header plus pathname is a multiple	of four
    current += namesize;
    int total_size = current - (char*) pos;
    int reminder = total_size % 4;
    if(reminder != 0){
      current  += (4 - reminder);
    }
    current += filesize;
    total_size = current - (char*) pos;
    reminder = total_size % 4;
    if(reminder != 0){
      current += (4 - reminder);
    }
  }
}

void cat_cmd(){
  uart_puts("Filename: ");
  uart_getline(recv_buf, RECV_LEN);
  struct cpio_newc_header *pos = (struct cpio_newc_header*)CPIO_BASE;
  char *current = (char*) CPIO_BASE;
  while(1){
    pos = (struct cpio_newc_header*)current;
    int namesize = _8byte_hexadecimal_to_int(pos->c_namesize);
    int filesize = _8byte_hexadecimal_to_int(pos->c_filesize);
    current += sizeof(struct cpio_newc_header);
    if(strcmp(current, "TRAILER!!!") == 0) {
      uart_puts("\r");
      uart_puts("File Not Found\n\r");
      break;
    }
    int output_flag = 0;
    if(strcmp(current, recv_buf) == 0){
      output_flag = 1;
    }
    current += namesize;
    int total_size = current - (char*) pos;
    int reminder = total_size % 4;
    if(reminder != 0){
      current  += (4 - reminder);
    }
    if(output_flag == 1){
      uart_puts("\r");
      uart_puts(current);
      break;
    }
    current += filesize;
    total_size = current - (char*) pos;
    reminder = total_size % 4;
    if(reminder != 0){
      current += (4 - reminder);
    }
  }
}

void shell(){
  while(1){
    uart_getline(recv_buf, RECV_LEN);
    if(strcmp(recv_buf, "ls") == 0){
      uart_puts("\r");
      ls_cmd();
    }
    else if(strcmp(recv_buf, "cat") == 0){
      uart_puts("\r");
      cat_cmd();
    }
    else if(strcmp(recv_buf, "hello") == 0){
      uart_puts("\rHELLO WORLD\n\r");
    }
    else {
      uart_puts("\rUknown cmd\n\r");
    }
  }
}


void main(){
  unsigned long DTB_BASE;
  asm volatile("mov %0, x20"
              :"=r"(DTB_BASE)
              );
  // set up serial console
    uart_init();
    
    fdt_traverse((fdt_header*)DTB_BASE, initramfs_callback);
    uart_puts("CPIO_BASE: ");
    uart_hex(CPIO_BASE);
    uart_puts("\n\r");
    
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
    char* string = simple_malloc(8);
    while(1) {
      shell();
    }
}