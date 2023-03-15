# OSC2023

| Github Account | Student ID | Name          |
|----------------|------------|---------------|
| Shin-Yan       | 310555008  | 曾信彥        |

## Requirements

* a cross-compiler for aarch64
* (optional) qemu-system-arm

## Build 

```
make
```

## Test With QEMU
place your dtb file in the img/ directory
```command
$ cp bcm2710-rpi-3-b-plus.dtb <osc_dir>/img/.
$ make qemuk
```
