# OSC2023

| Github Account | Student ID | Name          |
|----------------|------------|---------------|
| hunterhaha | 310833016    | 李翰寬 |

## Requirements

* a cross-compiler for aarch64
* (optional) qemu-system-arm

## Build 

```bash
make 
```

## Test With QEMU

```bash
qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial stdio
# or
make run
```
