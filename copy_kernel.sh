#!/bin/bash

mount /dev/sdb1 /mnt/osdi
cp $1 /mnt/osdi/
umount /dev/sdb1