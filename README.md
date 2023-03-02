# OSC2023

| Github Account | Student ID | Name          |
|----------------|------------|---------------|
| pin-chen | 109550206    | Pin-Shao Chen |

## Requirements

* a cross-compiler for aarch64
* (optional) qemu-system-arm

## Build 

```
make
```

## Test With QEMU

```
qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial stdio -display none
```
