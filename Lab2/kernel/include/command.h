#ifndef COMMAND_H
#define COMMAND_H

void input_buffer_overflow_message ( char [] );

void command_help ();
void command_hello ();
void command_not_found ( char * );
void command_reboot ();
void command_clear();

void command_info ();
void command_loading();
void command_ls();
void command_cat(char *args);
void command_dtb();




#endif