# lab1_311551131
311551131
# OSC2023

| Github Account | Student ID   | Name       |
|----------------|--------------|------------|
| psychicalcoder | 311551131    | Jun-Yu Lai |

## Requirements

* a cross-compiler for aarch64
* (optional) qemu-system-arm

## Build 

```
make kernel.img
```

## Test With QEMU

```
qemu-system-aarch64 -M raspi3b -kernel kernel.img -initrd initramfs.cpio -serial null -serial stdio -dtb bcm2710-rpi-3-b-plus.dtb
```
