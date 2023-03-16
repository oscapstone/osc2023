#include "shell.h"

void shell()
{
    char cmd[MAX_BUF_SIZE];

    while (1)
    {
        uart_printf("# ");
        uart_gets(cmd);
        cmd_resolve(cmd);
    }
}

void welcome()
{
    uart_printf("\x1b[H\x1b[J");
    uart_printf("Welcome to my budget kernel !!!\n");
    uart_printf("dtb address: %x\n", dtb_base);
}

void cmd_resolve(char *cmd)
{
    char *argv[100];
    unsigned int argc = 0;
    unsigned int cmd_length = strlen(cmd);
    int i, j;

    argv[0] = simple_malloc(MAX_BUF_SIZE);

    /* String processing */
    for (i = 0, j = 0; i < cmd_length; ++i)
    {
        if (cmd[i] == ' ')
        {
            argv[argc][j] = '\0';
            argv[++argc] = simple_malloc(MAX_BUF_SIZE);
            j = 0;
            continue;
        }
        argv[argc][j++] = cmd[i];
    }
    argv[argc++][j] = '\0';

    if (!strcmp(argv[0], "help"))
    {
        uart_printf("help                            : print this help menu\n");
        uart_printf("hello                           : print Hello World!\n");
        uart_printf("mailbox                         : show infos of board revision and ARM memory\n");
        uart_printf("ls                              : list directory contents\r\n");
        uart_printf("cat [FILE]                      : concatenate files and print on the standard output\r\n");
        uart_printf("reboot                          : reboot the device\n");
        uart_printf("clear                           : clear page\n");
    }
    else if (!strcmp(argv[0], "hello"))
        uart_printf("Hello World!\n");
    else if (!strcmp(argv[0], "reboot"))
        reboot();
    else if (!strcmp(argv[0], "mailbox"))
    {
        unsigned int board_revision;
        unsigned int base_addr;
        unsigned int size;

        if (get_board_revision(&board_revision) != -1)
            uart_printf("Board Revision: 0x%x\n", board_revision);
        
        if (get_arm_memory_info(&base_addr, &size) != -1)
            uart_printf("ARM memory base address: 0x%x\nARM memory size: 0x%x\n", base_addr, size);
    }
    else if (!strcmp(argv[0], "ls"))
        ls(".");
    else if (!strcmp(argv[0], "cat"))
        cat(argv[1]);
    else if (!strcmp(argv[0], "clear"))
        uart_printf("\x1b[H\x1b[J");
    else if (!strcmp(argv[0], "alloc"))
    {
        char *str = (char *)simple_malloc(8);
        *str = 'a';
        *(str + 1) = 'b';
        *(str + 2) = 'c';
        *(str + 3) = '\0';
        uart_printf("%s\n%x\n", str, __heap_top);
    }
    else
        uart_printf("Unknown command: %s\n", argv[0]);
}

