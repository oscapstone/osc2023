#include "uart.h"
#include "command.h"
#include "mailbox.h"
#include "reboot.h"
#include "string.h"
#include "allocate.h"
#include "fdt.h"
#include "timer.h"
#define RECV_LEN 100
#define USER_STACK_TOP 0x40000
static char recv_buf[RECV_LEN] = {0};

extern void from_el1_to_el0(unsigned int instr_addr, unsigned int stack_addr);

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

void exec_cmd(){
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
    int load_flag = 0;
    if(strcmp(current, recv_buf) == 0){
      load_flag = 1;
    }
    current += namesize;
    int total_size = current - (char*) pos;
    int reminder = total_size % 4;
    if(reminder != 0){
      current  += (4 - reminder);
    }
    if(load_flag == 1){
      uart_puts("\r");
      char *program_start = (void*)current;
      uart_puts("LOAD USER PROG\n\r");
      from_el1_to_el0((unsigned int)program_start, (unsigned int)USER_STACK_TOP);
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

void timer_print1_callback(char* msg){
  unsigned int sec;
  volatile unsigned int cntpct_el0;
  asm volatile("mrs %0, cntpct_el0"
               : "=r" (cntpct_el0));
  volatile unsigned long cntfrq_el0;
  asm volatile("mrs %0, cntfrq_el0"
               : "=r" (cntfrq_el0));
  
  sec = cntpct_el0 / cntfrq_el0;
  uart_puts("Second after boot: ");
  uart_hex(sec);
  uart_puts("\n\r");
  uart_puts(msg);
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
    else if(strcmp(recv_buf, "exec") == 0){
      uart_puts("\r");
      exec_cmd();
    }
    else if(strcmp(recv_buf, "async") == 0){
      uart_puts("\r");
      async_test();
    }
    else if(strcmp(recv_buf, "timer") == 0){
      uart_puts("\n");
      time_multiplex_test();
    }
    else if(strcmp(recv_buf, "reboot") == 0){
      uart_puts("\rReboot\n\r");
      reset(100);
    }
    else if(startwith(recv_buf, "set \"") == 0){
      uart_puts("CMD: settimeout\n\r");
      int msg_start = find_in_str(recv_buf, '"');
      int msg_end = msg_start + 1 + find_in_str(&recv_buf[msg_start + 1], '"');
      int int_start = msg_end + 2;
      int int_end = find_in_str(recv_buf, '\0') - 1;
      unsigned int size = msg_end - msg_start + 3;
      size += (size % 16);
      char*  msg = (char*) simple_malloc(32);
      msg_start ++;
      msg_end --;
      for(int i = 0; i < (msg_end - msg_start + 2); i++){
        if(i != (msg_end - msg_start + 1))
          *(msg + i) = recv_buf[msg_start + i];
        else{
          *(msg + i) = '\n';
          *(msg + i + 1) = '\r';
        }

      }
      //extract timout value
      unsigned long timeout = 0;
      for(int i=int_start; i <= int_end; i++){
        if(recv_buf[i] >= '0' && recv_buf[i] <= '9'){
          timeout *= 10;
          timeout += (recv_buf[i] - '0');
        }
        else{
          uart_puts("Timeout number error\n\r");
          continue;
        }
      }
      timeout = get_cpu_freq() * timeout;
      add_timer(timer_print1_callback, msg, timeout);
    }
    else if(strcmp(recv_buf, "help") == 0){
      uart_puts("help:\t\tlist available command\n\r");
      uart_puts("ls:\t\tlist initramfs files\n\r");
      uart_puts("cat:\t\tshow the file content\n\r");
      uart_puts("hello:\t\tprint \"hello world\"\n\r");
      uart_puts("exec:\t\tececute the user program\n\r");
      uart_puts("async:\t\ttestcase for async uart\n\r");
      uart_puts("timer:\t\ttestcase for timer multiplexing\n\r");
      uart_puts("reboot:\t\treboot rpi\n\r");
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
    asm volatile("msr DAIFClr, 0xf");
    core_timer_enable();
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
    while(1) {
      shell();
    }
}