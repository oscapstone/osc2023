# [2023 OSC] Lab5

## memory

After Kernel is loaded:

|       |Address                | Name          |
|:----  |---------------:       |:---------     |
|       |                       |kerel stack ⬆️ |
|low    |0x8_0000               |kernel ⬇️      |
|       |                       |(blank)        |
|       |0x2000_0000 (200MB)    |ramdisk ⬇️     |
|high   |(751MB+)               |devtree ⬇️     |

## Basic 1: Thread

- [Callee and caller-saved register](https://blog.csdn.net/l919898756/article/details/103142439)
- [Function pointer in C](https://www.scaler.com/topics/c/function-pointer-in-c/)
- [ARM FP 寄存器及 frame pointer 介绍](https://blog.csdn.net/tangg555/article/details/62231285)
- GDB tui

## Basic 2: User Process and System Call

- [StackOverflow: How to implement system call in ARM64?](https://stackoverflow.com/questions/25431095/how-to-implement-system-call-in-arm64)
- [Linux 系統程式設計 - read()、write() 與 page cache](https://blog.jaycetyle.com/2019/01/linux-read-write/)