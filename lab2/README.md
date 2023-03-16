# [2023 OSC] Lab2

[Spec](https://oscapstone.github.io/labs/lab2.html)

## Basic Exercise 1 - UART Bootloader

- [Booting Your Own Kernel on Raspberry Pi via Uart](https://blog.nicolasmesa.co/posts/2019/08/booting-your-own-kernel-on-raspberry-pi-via-uart/)

## Basic Exercise 2 - Initial Ramdisk

- [cpio man page](https://man.freebsd.org/cgi/man.cgi?query=cpio&sektion=5)

## Basic Exercise 3 - Simple Allocator

## Advanced Exercise 1 - Bootloader Self Relocation

- [Load addresses to a register using LDR Rd, =label](https://developer.arm.com/documentation/dui0473/m/writing-arm-assembly-language/load-addresses-to-a-register-using-ldr-rd---label)
- Branch
  - BR: Branch to register
  - BL: Branch with Link, branches to a PC-relative offset (can return)
  - B: Unconditional branch to a label at a PC-relative offset
  - B.cond: Branch conditionally ([arm指令之beq和bnq - csdn](https://blog.csdn.net/u014470361/article/details/88634509))
- ADR: Address of label at a PC-relative offset.
- [Show Register Values of QEMU Emulator](https://rip.hibariya.org/post/show-register-values-of-qemu-emulator/)
- [ARM Instruction Set](https://iitd-plos.github.io/col718/ref/arm-instructionset.pdf)

- |        |Memory Layout|
  |--------|:------:|
  |**high**|environment variables and command line arguments|
  |        |stack|
  |        |:arrow_down:|
  |        |:white_small_square:|
  |        |:arrow_up:|
  |        |heap|
  |        |uninitialized data(bss)|
  |        |initialized data|
  |**low** |text|


## Advanced Exercise 2 - Devicetree

- [devicetree-specification](https://github.com/devicetree-org/devicetree-specification/releases/tag/v0.4-rc1)
- [Parameters in general-purpose registers](https://developer.arm.com/documentation/den0024/a/The-ABI-for-ARM-64-bit-Architecture/Register-use-in-the-AArch64-Procedure-Call-Standard/Parameters-in-general-purpose-registers)
- [Inline Assembly & Memory Barrier](https://ithelp.ithome.com.tw/articles/10213500)
- [How does the linux kernel know about the initrd when booting with a device tree?](https://stackoverflow.com/questions/73974443/how-does-the-linux-kernel-know-about-the-initrd-when-booting-with-a-device-tree)