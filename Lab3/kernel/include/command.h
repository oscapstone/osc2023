#ifndef COMMAND_H
#define COMMAND_H



/* Command */

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
void command_exec(char *args);
void command_el();
void command_setTimeout(char *args);
void command_set2sAlert();
#endif