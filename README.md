# OSC2023

| Github Account | Student ID | Name          |
|----------------|------------|---------------|
| a15923647      | 109550043  | Jian-Zhe Wang |

## Requirements

* a cross-compiler for aarch64
* qemu-system-arm

## Environment setup
```
./env_setup.sh
```

## Build 

```
make
```

## Test With QEMU
Test with qemu.
```
make test
```

Then, open another terminal and run gdb with script.
```
sudo gdb-multiarch -x gdb_test_with_qemu.txt
```
## Lab1 Hello World
[Spec](https://oscapstone.github.io/labs/lab1.html)
### Basic Exercise 1 - Basic Initialization
Refer to the [rpi os tutorial](https://github.com/s-matyukevich/raspberry-pi-os/blob/master/docs/lesson01/rpi-os.md#booting-the-kernel).\
When a program is loaded, it requires,
* All it’s data is presented at correct memory address. 
    * done by linker script
    * [tutorial](https://blog.louie.lu/2016/11/06/10%E5%88%86%E9%90%98%E8%AE%80%E6%87%82-linker-scripts/)
* The program counter is set to correct memory address.
    * jump to main
* The bss segment are initialized to 0.
    * use my_bzero
* The stack pointer is set to a proper address.
    * which should be far enough from text section
For more detail implementation, have a look at src/boot.s.

### Basic Exercise 2 - Mini UART
[tutorial](https://github.com/s-matyukevich/raspberry-pi-os/blob/master/docs/lesson01/rpi-os.md#raspberry-pi-devices)
#### Initialization
Overall steps include
* configure GPFSELn register to change alternate function
* configure pull up/down register to disable GPIO pull up/down
#### Read data
1. Check AUX_MU_LSR_REG’s data ready field.
2. If set, read from AUX_MU_IO_REG
#### Write data
1. Check AUX_MU_LSR_REG’s Transmitter empty field.
2. If set, write to AUX_MU_IO_REG
**By default, QEMU uses UART0 (PL011 UART) as serial io. If you want to use UART1 (mini UART) use flag -serial null -serial stdio**
For more detail implementation, have a look at src/uart.c and include/uart.h.

### Basic Exercise 3 - Simple Shell
Implement a simple shell to
* help: print all available commands
* hello: print Hello World!

**Should handle \r\n on both input and output.**
Utilize functions created in Basic Exercise 2 - Mini UART to read input and output result.\
The implementation supports to edit command dynamically.\
For more details, have a look at src/main.c and src/shell.c

### Basic Exercise 4 - Mailbox
Mailbox is a communication mechanism used between the Raspberry Pi's ARM CPU and its VideoCore IV GPU.
[mailbox format](https://jsandler18.github.io/extra/mailbox.html)
#### Get the hardware’s information
Get the hardware’s information is the easiest thing could be done by mailbox. \
Check if you implement mailbox communication correctly by verifying the hardware information’s correctness.\
This include
1. fill in information to a buffer called mailbox
2. pass messages by the mailbox (mailbox_call function is defined in src/mailbox.c)
    1. Combine the message address (upper 28 bits) with channel number (lower 4 bits)
    2. Check if Mailbox 0 status register’s full flag is set.
    3. If not, then you can write to Mailbox 1 Read/Write register.
    4. Check if Mailbox 0 status register’s empty flag is set.
    5. If not, then you can read from Mailbox 0 Read/Write register.
    6. Check if the value is the same as you wrote in step 1.
### Advanced Exercise 1 - Reboot
The reboot command is treated as reset(0) in following code.
```c
#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024

void set(long addr, unsigned int value) {
    volatile unsigned int* point = (unsigned int*)addr;
    *point = value;
}

void reset(int tick) {                 // reboot after watchdog timer expire
    set(PM_RSTC, PM_PASSWORD | 0x20);  // full reset
    set(PM_WDOG, PM_PASSWORD | tick);  // number of watchdog tick
}

void cancel_reset() {
    set(PM_RSTC, PM_PASSWORD | 0);  // full reset
    set(PM_WDOG, PM_PASSWORD | 0);  // number of watchdog tick
}
```