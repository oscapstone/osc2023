# /bin/bash

qemu-system-aarch64 -machine raspi3b \
                    -cpu cortex-a53  \
                    -serial null -serial stdio \
                    -kernel ./kernel8.img \
