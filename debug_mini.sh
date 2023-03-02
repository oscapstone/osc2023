#!/bin/bash
# make && qemu-system-aarch64 -M raspi3b    -cpu cortex-a72    -dtb bcm2710-rpi-3-b-plus.dtb    -m 1G -smp 4 -serial null -serial stdio -kernel kernel8.img -display none -S -s
SESSION_NAME=debug_mini
make && tmux new-session -d -s $SESSION_NAME

tmux rename-window -t $SESSION_NAME:0 "main"
tmux split-window -v -p 50 -t $SESSION_NAME:main
tmux send-keys -t $SESSION_NAME:main.0 "qemu-system-aarch64 -M raspi3b    -cpu cortex-a72    -dtb bcm2710-rpi-3-b-plus.dtb    -m 1G -smp 4 -serial null -serial stdio -kernel kernel8.img -display none -S -s" C-m
tmux send-keys -t $SESSION_NAME:main.1 "gdb-multiarch build/kernel8.elf" C-m
tmux send-keys -t $SESSION_NAME:main.1 "set archi aarch64" C-m
tmux send-keys -t $SESSION_NAME:main.1 "target remote :1234" C-m
tmux attach -t $SESSION_NAME
