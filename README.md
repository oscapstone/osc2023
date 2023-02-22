# OSC2023

| Github Account | Student ID | Name          |
|----------------|------------|---------------|
| a15923647      | 109550043  | Jian-Zhe Wang |

## Requirements

* a cross-compiler for aarch64
* (optional) qemu-system-arm

## Environment setup
```
./env_setup.sh
```

## Build 

```
make kernel.img
```

## Test With QEMU
Run qemu.
```
make test
```

Open another terminal and run gdb with script.
```
sudo gdb-multiarch -x gdb_test_with_qemu.txt
```