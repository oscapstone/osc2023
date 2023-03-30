# OSC_Lab

NYCU 2023 Operating System Capstone

## Whoami

- Github Account Name: @ShangHungWan
- Student ID: 311551182
- Name: 萬尚宏

## Environment

- Fedora 37
- QEMU 7.0.0
- Raspbery Pi 3B+

## How to use

### Install Dependencies

```shell
sudo apt update
sudo apt install qemu-system-aarch64 gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu -y
```

### Build Library

```shell
cd library && make && cd ..
```

### Run on QEMU

```sehll
# bootloader
cd bootloader && make && make run

# or kernel
cd kernel && make && make run 
```

## Version History

### 2023-02-28

Finish Lab 1

- Basic
    - [x] Basic Initialization
    - [x] Mini UART
    - [x] Simple Shell
    - [x] Mailbox
- Advanced
    - [x] Reboot

### 2023-03-28

Finish Lab 2

- Basic
    - [x] UART Bootloader
    - [x] Initial Ramdisk
    - [x] Simple Allocator
- Advanced
    - [x] Bootloader Self Relocation
    - [x] Devicetree
