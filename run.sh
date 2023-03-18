#!/bin/bash
read QEMU_PTY \
    < <(qemu-system-aarch64 -M raspi3b -kernel $1 -daemonize -display none -serial null -serial pty -initrd $3 \
    | awk '{print $5}')
echo "QEMU serial port is attached to $QEMU_PTY"

tmux new -d \; \
     splitw -h \; \
     splitw -v \; \
     selectp -t 0 \; \
     send "screen $QEMU_PTY 115200" C-m \; \
     selectp -t 2 \; \
     send 'clear' C-m \; \
     selectp -t 1 \; \
     send 'bash' C-m "clear; while true; do read -p 'Press enter to load kernel ...'; ./$2 $QEMU_PTY; done" C-m \; \
     attach

read QEMU_DEAMON \
    < <(ps -eo 'tty ppid pid command' \
    | grep -i '^?.*qemu' \
    | awk '{print $3}')

kill -9 $QEMU_DEAMON
echo "QEMU deamon is killed (pid: $QEMU_DEAMON)"