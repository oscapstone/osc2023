# OS

Physical Environment: Raspberry Pi 3b+

Simulate Environment: QEMU

Build Command:
```
make
```

Simulate Command:
```
qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial stdio -display none
```
