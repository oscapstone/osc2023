# [2023 OSC] Lab4

## memory

After Kernel is loaded:

|       |Address                | Name          |
|:----  |---------------:       |:---------     |
|       |                       |kerel stack ⬆️ |
|low    |0x8_0000               |kernel ⬇️      |
|       |                       |(blank)        |
|       |0x2000_0000 (200MB)    |ramdisk ⬇️     |
|high   |(751MB+)               |devtree ⬇️     |