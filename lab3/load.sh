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
# path="/dev/pts/5"
kernel="./kernel8.img"

# Get the file size
# wc -c $kernel | sed 's/ .*//' | tr -d '\n' > $path
wc -c $kernel | sed 's/ .*//' > $path

# Avoid binary drop
sleep 1

# Write kernel
# cat $kernel | pv --quiet --rate-limit 100 > $path
cat $kernel > $path
