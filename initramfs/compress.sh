#!/bin/bash

#use "apt-get uboot-mkimage" for the mkimage tool

#sdcard output partition mount point (will put SmartQ5 image file in)
SDPART=/media/2B3F-7268

#kernel to use
#ZIMAGE=./zimage
ZIMAGE=./zImageSD
#ZIMAGE=./zImageMer
#ZIMAGE=./zImage2631q
#ZIMAGE=./zImage34
#ZIMAGE=./zImage2635

rm initrd.igz

#repair lost rights in git
./prepare-dev.sh
chmod +s content/bin/busybox
chmod +x content/sbin/init

(cd content; find . -print | cpio -o --format=newc --owner=0) | gzip -c -9 > tmp.gz

cp tmp.gz ../qi-smartq/kernel-wrap/initrd.gz

mkimage -d tmp.gz -A ARM -O Linux -C gzip -T RAMDisk -n "SmartQ initramfs" initrd.igz

../fw-utils/mkSmartQ5 no-qi.nb0 no-u-boot.bin $ZIMAGE initrd.igz

[ -f $SDPART/SmartQ5 ] && rm $SDPART/SmartQ5
cp SmartQ5 $SDPART/

umount $SDPART
