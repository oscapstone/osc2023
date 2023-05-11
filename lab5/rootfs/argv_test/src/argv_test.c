#include "printf.h"
#include "sys.h"
#include "uart.h"

int main(int argc, char **argv) 
{
    init_printf(0, putc);

    // printf("\n[exec_argv_test]Argv Test, pid %d\n", getpid());
    // for (int i = 0; i < argc; ++i) {
    //     printf("%s\n", argv[i]);
    // }

    printf("Init success\n");
    
    uartwrite("Successfully\n", 14);

    char *fork_argv[] = {"fork_test", 0};
    exec("fork_test.img", fork_argv);

    exit(); // should never run

    return 0;
}
