#!/bin/bash
#Use:	pass kernel8.img to /dev/ttyUSB

file_name=../kernel8.img 
test ! -e $file_name && echo "../kernel8.img not exist!!" && exit 0
wc -c ../kernel8.img | awk {'printf("%s;",$1)'} > /dev/ttyUSB8
sleep 1
cat ../kernel8.img | pv -q -L 100 > /dev/ttyUSB8
