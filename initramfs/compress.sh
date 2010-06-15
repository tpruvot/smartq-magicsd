#!/bin/sh

#use "apt-get uboot-mkimage" for the mkimage tool

#sdcard output partition mount point (will put SmartQ5 image file in)
SDPART=/media/2B3F-7268

#kernel to use
ZIMAGE=./zImageMer

rm initrd.igz

(cd content; find . -print | cpio -o --format=newc --owner=0) | gzip -c -9 > tmp.gz

mkimage -d tmp.gz -A ARM -O Linux -C gzip -T RAMDisk -n "SmartQ initramfs" initrd.igz

../fw-utils/mkSmartQ5 qi.nb0 u-boot.bin $ZIMAGE initrd.igz

rm $SDPART/SmartQ5
cp SmartQ5 $SDPART/
