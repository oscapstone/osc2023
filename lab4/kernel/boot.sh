#!/bin/bash

set -e

if [ "$EUID" -ne 0 ]
then
	echo "Please run as root!"
	exit
fi

path="/dev/ttyUSB0"
kernel="./kernel8.img"

# Avoid binary drop
sleep 1

# Write kernel
cat $kernel | pv --quiet --rate-limit 100 > $path

