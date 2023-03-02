# Operating Systems Capstone 2023 Lab
- Github account name: GalenWang2017
- Student ID: 311554040
- Student name: 王璟倫

---
#### Make kernel image
```Shell
make
```
---
#### Test with QEMU
```Shell
qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -serial null -serial stdio
```