# OSC2023

| Github Account | Student ID | Name          |
|----------------|------------|---------------|
| foodieteng | A121164    | Chun-Teng Chang |

## Requirements

* a cross-compiler for aarch64
* (optional) qemu-system-arm

## Build 

```
make
```

## Test With QEMU

```
qemu-system-aarch64 -M raspi3b -serial null -serial stdio -display none  -kernel kernel8.img 
```
