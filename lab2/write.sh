#!/bin/bash

# This script for writing kernel to raspi
# Note: the speed of the kernel write need 
#	to be limit, or the buffer on raspi
#	will overflow.

set -e

if [ "$EUID" -ne 0 ]
then
	echo "Please run as root!"
	exit
fi

path="/dev/ttyUSB0"
kernel="./kernel.img"

# Get the file size
wc -c ./kernel.img | sed 's/ .*//' | tr -d '\n' > /dev/ttyUSB0

# Avoid binary drop
sleep 1

# Write kernel
cat $kernel | pv --quiet --rate-limit 100 > $path




# Reference: https://superuser.com/questions/526242/cat-file-to-terminal-at-particular-speed-of-lines-per-second

