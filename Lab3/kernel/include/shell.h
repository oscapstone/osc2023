#ifndef SHELL_H
#define SHELL_H

#define MAX_BUFFER_LEN 128

enum SPECIAL_CHARACTER
{
    BACK_SPACE = 8,
    LINE_FEED = 10,
    CARRIAGE_RETURN = 13,
    DELETE =  127,

    
    REGULAR_INPUT = 1000,
    NEW_LINE = 1001,
    
    UNKNOWN = -1,

};
enum SPECIAL_CHARACTER parse ( char );


void shell_start () ;
void welcome_msg() ;
void command_controller ( enum SPECIAL_CHARACTER, char c, char [], int *);
void input_buffer_overflow_message ( char [] );


#endif