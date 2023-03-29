# [2023 OSC] Lab3

## Basic Exercise 1 - Exception - 30%

### SPSR_EL2, Saved Program Status Register (EL2)
[DOCUMENT](https://developer.arm.com/documentation/ddi0601/2020-12/AArch64-Registers/SPSR-EL2--Saved-Program-Status-Register--EL2-)

D, bit [9]
Debug exception mask. Set to the value of PSTATE.D on taking an exception to EL2, and copied to PSTATE.D on executing an exception return operation in EL2.

On a Warm reset, this field resets to an architecturally UNKNOWN value.

A, bit [8]
SError interrupt mask. Set to the value of PSTATE.A on taking an exception to EL2, and copied to PSTATE.A on executing an exception return operation in EL2.

On a Warm reset, this field resets to an architecturally UNKNOWN value.

I, bit [7]
IRQ interrupt mask. Set to the value of PSTATE.I on taking an exception to EL2, and copied to PSTATE.I on executing an exception return operation in EL2.

On a Warm reset, this field resets to an architecturally UNKNOWN value.

F, bit [6]
FIQ interrupt mask. Set to the value of PSTATE.F on taking an exception to EL2, and copied to PSTATE.F on executing an exception return operation in EL2.

On a Warm reset, this field resets to an architecturally UNKNOWN value.

Bit [5]
Reserved, RES0.

### 

- [GDB 基礎操作](https://ithelp.ithome.com.tw/articles/10235522)
- [How to Debug C Program using gdb in 6 Simple Steps](https://u.osu.edu/cstutorials/2018/09/28/how-to-debug-c-program-using-gdb-in-6-simple-steps/)
- [Show current assembly instruction in GDB](https://stackoverflow.com/questions/1902901/show-current-assembly-instruction-in-gdb)
- [ARM64的启动过程之（六）：异常向量表的设定](http://www.wowotech.net/armv8a_arch/238.html)

## Basic Exercise 2 - Interrupt - 10%

## Basic Exercise 3 - Rpi3’s Peripheral Interrupt - 30%

- [bcm2836-peripherals](https://datasheets.raspberrypi.com/bcm2836/bcm2836-peripherals.pdf)

## Advanced Exercise 1 - Timer Multiplexing - 20%

- [Linux GDB](https://wangchujiang.com/linux-command/c/gdb.html)

## Advanced Exercise 2 - Concurrent I/O Devices Handling 20%

## memory

After Kernel is loaded:

|       |Address        | Name          |
|:----  |----------:    |:---------     |
|low    | 0x80000       |kernel ⬇️      |
|       |0x400000       |kerel stack ⬆️ |
|       |               |(blank)        |
|       |200MB          |ramdisk ⬇️     |
|high   |(751MB+)       |devtree ⬇️     |

## Others

(chatGPT)
An auxiliary (AUX) interrupt is a type of interrupt that occurs in a computer system when a device or component sends a signal to the CPU requesting attention.

An AUX interrupt is typically used for non-critical tasks that require occasional attention from the CPU, such as keyboard input, mouse movements, or other user interactions. The interrupt handler for an AUX interrupt is designed to quickly process the interrupt and return control to the main program.

In general, interrupts are an important mechanism for managing and coordinating the operation of different parts of a computer system. By using interrupts, devices can communicate with the CPU without requiring constant polling, which can improve performance and reduce power consumption.