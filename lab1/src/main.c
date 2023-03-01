#include "uart.h"
#include "utils.h"


int main() {
	shell_init();
	char cmd[100];
	int status=0;
	while (1) {
		switch (status) {
			case 0:
				shell_input(cmd);
				status = 1;
				break;
			case 1:
				shell_command(cmd);
				status = 0;
				break;
		}
	}
}


