#!/bin/bash

# Use this script to recompress a SmartQ5 image with manually extracted initramfs.igz

#sdcard output partition mount point (will put SmartQ5 image file in)
SDPART=/media/2B3F-7268

#kernel to use
ZIMAGE=./zimage

../fw-utils/mkSmartQ5 no-qi.nb0 no-u-boot.bin $ZIMAGE initramfs.igz

rm $SDPART/SmartQ5
cp SmartQ5 $SDPART/

umount $SDPART
