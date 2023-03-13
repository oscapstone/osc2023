#ifndef COMMAND_H
#define COMMAND_H

void input_buffer_overflow_message ( char [] );

void command_help ();
void command_hello ();
void command_timestamp ();
void command_not_found ( char * );
void command_reboot ();
void command_info ();
void command_clear();
void command_loading();
void command_cpio_list();
void command_cpio_cat(char *args);
void command_dt_info();
void command_parse_dt();
void command_dtb();




#endif