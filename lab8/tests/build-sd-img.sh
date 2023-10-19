#!/bin/sh

wget https://github.com/GrassLab/osdi/raw/master/supplement/sfn_nctuos.img \
    -O sd.img

img_sz=$(du -b sd.img | cut -f 1)

# Round img_sz to the next power of 2.
img_sz=$((img_sz - 1))
img_sz=$((img_sz | (img_sz >> 1)))
img_sz=$((img_sz | (img_sz >> 2)))
img_sz=$((img_sz | (img_sz >> 4)))
img_sz=$((img_sz | (img_sz >> 8)))
img_sz=$((img_sz | (img_sz >> 16)))
img_sz=$((img_sz + 1))

qemu-img resize -f raw sd.img $img_sz

wget https://oscapstone.github.io/_downloads/10fbdc3e04b471849e714edcdcf4ce26/FAT_R.TXT

echo
echo 'Now, manually put FAT_R.TXT into sd.img.'
