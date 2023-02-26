# My OSDI 2023 - LAB 1

## Author
| Student ID | Name | GitHub Account | Email                      |
| -----------| ---- | -------------- | -------------------------- |
| 311555009  | 呂義信|  Richard-YH    | luyh@cs.nctu.edu.tw        |


## Files Information
```bash
.
├── include       # header files
│   ├── command.h     # header file for processing commands.
│   ├── gpio.h        # header file for defining constant addresses.
│   ├── type.h        # header file for implementing certain functions found in the standard <type.h> header file.
│   ├── mailbox.h     # header file for processing mailboxes.
│   ├── math.h        # header file for implementing certain functions found in the standard <math.h> header file.
│   ├── shell.h       # header file for processing shell flow.
│   ├── string.h      # header file for implementing certain functions found in the standard <string.h> header file.
│   ├── time.h        # header file for processing functions related to time
│   ├── type.h        # header file for define some type like in <type.h>
│   └── uart.h        # header file for process uart interface
├── src          # source files
│   ├── command.c     # source code for processing commands
│   ├── mailbox.c     # source code for processing mailbox
│   ├── main.c        # source code for main function
│   ├── math.c        # source code for implementing functions in <math.h>
│   ├── shell.c       # source code for processing shell flow
│   ├── start.S       # source code for booting
│   ├── string.c      # source code for implementing functions in <string.h>
│   └── uart.c        # source code for processing UART interface
├── obj           # object file
├── link.ld       # linker script
├── Makefile      # makefile
└── README.md


```



## How to build

```bash
make
```

## Run on QEMU
```bash
make run
```

## Run Debug
```bash
make debug
```
To debug, I employ an Ubuntu container and begin by executing the following command
```bash
docker exec -it pwn_test /bin/bash
gdb kernel8.elf
target remote localhost:1234
```





## Simple Shell
| Command   | Description                   | 
| ----------| ----------------------------- | 
| hello     | print Hello World!            |
| help      | print all available commands  |
| info      | print board revision and ARM memory base address and size                            |
| reboot    | reboot                        |
| clear     | clean screen                  |