#ifndef COMMAND_H
#define COMMAND_H

void input_buffer_overflow_message ( char [] );

void command_help ();
void command_not_found ( char * );
void command_reboot ();
void command_loadimg ();
void command_clear();

#endif