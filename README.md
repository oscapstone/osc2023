# OSC2023

| Github Account | Student ID | Name          |
|----------------|------------|---------------|
| RobertttBS     | 311555026  | 謝柏陞         |

## Requirements

* Initialize rpi3 after booted by bootloader.
* Follow UART to set up mini UART.
* Implement a simple shell supporting the listed commands.
* Get the hardware’s information by mailbox and print them, you should at least print board revision and ARM memory base address and size.
* Add a <reboot> command.

## Build 

```
make
```

## Test With QEMU

```
make run
# or 
qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial stdio
```
