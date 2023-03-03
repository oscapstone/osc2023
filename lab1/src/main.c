#include "shell.h"

enum status{
	Read, Control
};

void main()
{
    shell_init();
	enum status s = Read;
	
    while(1) {
		char cmd[128];
		if (s == Read){
			shell_input(cmd);
			s = Control;
		}
		else{
			shell_controller(cmd);
			s = Read;
		}

    }
}