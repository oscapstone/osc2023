# [2023 OSC] Lab1

[Spec](https://oscapstone.github.io/labs/lab1.html)

## Basic Initialization

### Makefile

- `Wall` Show all warnings.
- `nostdlib` Don't use the C standard library. Most of the calls in the C standard library eventually interact with the operating system. We are writing a bare-metal program, and we don't have any underlying operating system, so the C standard library is not going to work for us anyway.
- `nostartfiles` Don't use standard startup files. Startup files are responsible for setting an initial stack pointer, initializing static data, and jumping to the main entry point. We are going to do all of this by ourselves.
- `ffreestanding` A freestanding environment is an environment in which the standard library may not exist, and program startup may not necessarily be at main. The option -ffreestanding directs the compiler to not assume that standard functions have their usual definition.
- `Iinclude` Search for header files in the include folder.
- `mgeneral-regs-only`. Use only general-purpose registers. ARM processors also have NEON registers. We don't want the compiler to use them because they add additional complexity (since, for example, we will need to store the registers during a context switch).

### Qemu Commands

- `qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial stdio`
- [qemu-system-aarch64(1) - Debian Manpages](https://manpages.debian.org/testing/qemu-system-arm/qemu-system-aarch64.1.en.html)

### Linker
[src/linker.ld](./src/linker.ld)

- **BSS**: If the section is not aligned, it would be more difficult to use the `str` instruction to store 0 at the beginning of the `bss` section because the `str` instruction can be used only with 8-byte-aligned addresses.
- [10分鐘讀懂 linker scripts](https://blog.louie.lu/2016/11/06/10%E5%88%86%E9%90%98%E8%AE%80%E6%87%82-linker-scripts/)

### Booting
[src/boot.S](./src/boot.S)

- ARM assembler syntax
    - **[mrs](http://www.keil.com/support/man/docs/armasm/armasm_dom1361289881374.htm)** Load value from a system register to one of the general purpose registers (x0–x30)
    - **[and](http://www.keil.com/support/man/docs/armasm/armasm_dom1361289863017.htm)** Perform the logical AND operation. We use this command to strip the last byte from the value we obtain from the `mpidr_el1` register.
    - **[cbz](http://www.keil.com/support/man/docs/armasm/armasm_dom1361289867296.htm)** Compare the result of the previously executed operation to 0 and jump (or `branch` in ARM terminology) to the provided label if the comparison yields true.
    - **[b](http://www.keil.com/support/man/docs/armasm/armasm_dom1361289863797.htm)** Perform an unconditional branch to some label.
    - **[adr](http://www.keil.com/support/man/docs/armasm/armasm_dom1361289862147.htm)** Load a label's relative address into the target register. In this case, we want pointers to the start and end of the `.bss` region.
    - **[sub](http://www.keil.com/support/man/docs/armasm/armasm_dom1361289908389.htm)** Subtract values from two registers.
    - **[bl](http://www.keil.com/support/man/docs/armasm/armasm_dom1361289865686.htm)** "Branch with a link": perform an unconditional branch and store the return address in x30 (the link register). When the subroutine is finished, use the `ret` instruction to jump back to the return address.
    - **[mov](http://www.keil.com/support/man/docs/armasm/armasm_dom1361289878994.htm)** Move a value between registers or from a constant to a register.

## Mini UART
[src/mini_uart.c](./src/mini_uart.c)

### Initialize Mini UART

- Follows [this article](https://oscapstone.github.io/labs/hardware/uart.html) provided by TAs
- **UART** stands for Universal asynchronous receiver-transmitter

### GPIO Alternative Function

### GPIO Pull-Up/Down

- **GPIO** stands for General-purpose input/output
- If you use a particular pin as input and don't connect anything to this pin, you will not be able to identify whether the value of the pin is 1 or 0. In fact, the device will report random values. The pull-up/pull-down mechanism allows you to overcome this issue. If you set the pin to the pull-up state and nothing is connected to it, it will report `1` all the time (for the pull-down state, the value will always be 0)
- There are three available states: pull-up, pull-down, and neither (to remove the current pull-up or pull-down state), and we need the third one.

## Simple Shell
[src/shell.c](./src/shell.c)

## Mailbox
[src/mailbox.c](./src/mailbox.c)

- Follows [this article](https://oscapstone.github.io/labs/hardware/mailbox.html) provided by TAs

## Reboot

- [Get Arm memory](https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface#get-arm-memory)
- [Get board revision](https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface#get-board-revision)

## Reference

- s-matyukevich/raspberry-pi-os Lesson 1 ([link](https://github.com/s-matyukevich/raspberry-pi-os/blob/master/docs/lesson01/rpi-os.md))
- bztsrc/raspi3-tutorial Tutorial 04 - Mailboxes ([link](https://github.com/bztsrc/raspi3-tutorial/tree/master/04_mailboxes#tutorial-04---mailboxes))
- [Linux kernel coding style](https://www.kernel.org/doc/html/v5.18/process/coding-style.html)
- [Assembly Language Coding Guidelines](https://projectacrn.github.io/latest/developer-guides/asm_coding_guidelines.html#asm-cs-05-shall-not-be-used-for-comments)